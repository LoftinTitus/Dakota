#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <string_view>
#include <cstdint>
#include <memory>

namespace Dakota {

// Constants for parser configuration
constexpr size_t MAX_STATE_STACK_DEPTH = 256;  // Prevent stack overflow
constexpr size_t MAX_MATRIX_DIMENSIONS = 1000;  // Reasonable matrix size limit
constexpr size_t DEFAULT_NODES_CAPACITY = 2048;  // Initial node capacity
constexpr size_t DEFAULT_STACK_CAPACITY = 128;   // Initial stack capacity

// Matrix validation error types
enum class MatrixError : uint8_t {
    NONE = 0,
    EMPTY_MATRIX,
    IRREGULAR_ROWS,
    MISSING_CLOSING_BRACKET,
    INVALID_ELEMENT,
    DIMENSION_MISMATCH
};

// Forward declarations
struct ASTNode;
struct ParseContext;

// AST Node Types - using enum for fast switching
enum class NodeType : uint8_t {
    INVALID = 0,
    
    // Literals
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    
    // Identifiers
    IDENTIFIER,
    
    // Binary operations
    BINARY_OP,
    
    // Unary operations
    UNARY_OP,
    
    // Assignment
    ASSIGNMENT,
    
    // Matrix operations
    MATRIX_LITERAL,
    MATRIX_MULTIPLY,
    MATRIX_ACCESS,
    
    // Array and member access
    ARRAY_ACCESS,
    MEMBER_ACCESS,
    
    // Control flow
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    
    // Functions
    FUNCTION_DEF,
    FUNCTION_CALL,
    RETURN_STATEMENT,
    
    // Blocks
    BLOCK,
    EXPRESSION_STATEMENT,
    
    // Program root
    PROGRAM
};

// Operator types for binary operations
enum class BinaryOpType : uint8_t {
    ADD, SUB, MUL, DIV, POW, MATMUL,
    EQ, NE, LT, LE, GT, GE,
    AND, OR
};

// Operator types for unary operations
enum class UnaryOpType : uint8_t {
    NEGATE, NOT
};

// Flat AST node structure - all nodes use this same struct
// This enables contiguous storage and cache-friendly access
struct ASTNode {
    NodeType type;
    uint32_t token_index;       // Index into original token array
    uint32_t parent_index;      // Index of parent node (0 = root)
    uint32_t first_child_index; // Index of first child (0 = no children)
    uint32_t next_sibling_index; // Index of next sibling (0 = no siblings)
    
    // Union for type-specific data - keeps struct size minimal
    union {
        struct {
            int64_t value;
        } integer_literal;
        
        struct {
            double value;
        } float_literal;
        
        struct {
            uint32_t string_index;  // Index into string table
            uint32_t length;
        } string_literal;
        
        struct {
            bool value;
        } boolean_literal;
        
        struct {
            uint32_t name_index;    // Index into string table
        } identifier;
        
        struct {
            BinaryOpType op_type;
            uint32_t left_index;    // Index of left operand
            uint32_t right_index;   // Index of right operand
        } binary_op;
        
        struct {
            UnaryOpType op_type;
            uint32_t operand_index;
        } unary_op;
        
        struct {
            uint32_t target_index;  // Index of assignment target
            uint32_t value_index;   // Index of assigned value
        } assignment;
        
        struct {
            uint32_t rows;
            uint32_t cols;
            uint32_t elements_start_index; // Index of first element
            MatrixError validation_error;  // Track validation state
            bool is_empty;                 // Handle empty matrix case
        } matrix_literal;
        
        struct {
            uint32_t object_index;  // Index of matrix being accessed
            uint32_t index_index;   // Index of access index
        } matrix_access;
        
        struct {
            uint32_t object_index;  // Index of array being accessed
            uint32_t index_index;   // Index of access index
        } array_access;
        
        struct {
            uint32_t object_index;  // Index of object
            uint32_t member_name_index; // Index of member name in string table
        } member_access;
        
        struct {
            uint32_t condition_index;
            uint32_t then_block_index;
            uint32_t else_block_index; // 0 if no else
        } if_statement;
        
        struct {
            uint32_t condition_index;
            uint32_t body_index;
        } while_statement;
        
        struct {
            uint32_t variable_index;    // Loop variable
            uint32_t iterable_index;    // Expression to iterate over
            uint32_t body_index;        // Loop body
        } for_statement;
        
        struct {
            uint32_t name_index;        // Function name
            uint32_t params_start_index; // First parameter
            uint32_t body_index;        // Function body
            uint32_t param_count;
        } function_def;
        
        struct {
            uint32_t name_index;        // Function name
            uint32_t args_start_index;  // First argument
            uint32_t arg_count;
        } function_call;
        
        struct {
            uint32_t value_index;       // 0 if void return
        } return_statement;
        
        struct {
            uint32_t statements_start_index;
            uint32_t statement_count;
        } block;
        
        struct {
            uint32_t expression_index;
        } expression_statement;
    };
    
    // Constructor for easy initialization
    ASTNode(NodeType t = NodeType::INVALID, uint32_t token_idx = 0)
        : type(t), token_index(token_idx), parent_index(0), 
          first_child_index(0), next_sibling_index(0) {
        // Initialize union to zero
        integer_literal.value = 0;
    }
};

// String table for efficient string storage
class StringTable {
private:
    std::vector<char> data;
    std::vector<uint32_t> offsets;
    
public:
    // Add string and return index
    uint32_t add_string(std::string_view str);
    
    // Get string by index
    std::string_view get_string(uint32_t index) const;
    
    // Get string data pointer (for zero-terminated strings)
    const char* get_c_string(uint32_t index) const;
    
    // Clear all strings
    void clear();
    
    // Get memory usage statistics
    size_t memory_usage() const { return data.size() + offsets.size() * sizeof(uint32_t); }
    
    // Memory optimization - shrink to fit after parsing
    void optimize_memory() {
        data.shrink_to_fit();
        offsets.shrink_to_fit();
    }
    
    // Statistics for debugging
    size_t get_string_count() const { return offsets.size(); }
    size_t get_data_size() const { return data.size(); }
};

// Parser state for iterative parsing
enum class ParseState : uint8_t {
    PROGRAM,
    STATEMENT,
    EXPRESSION,
    BINARY_OP,
    UNARY_OP,
    PRIMARY,
    MATRIX_LITERAL,
    FUNCTION_CALL,
    BLOCK,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    FUNCTION_DEF,
    PARAMETER_LIST,
    ARGUMENT_LIST,
    ASSIGNMENT
};

// Operator precedence table
struct OperatorInfo {
    BinaryOpType op_type;
    uint8_t precedence;
    bool right_associative;
};

// Parse context for iterative parsing
struct ParseContext {
    const std::vector<Token>& tokens;
    size_t current_token;
    size_t token_count;
    
    // Pre-allocated AST storage
    std::vector<ASTNode> nodes;
    StringTable strings;
    
    // Parse state stack for iterative parsing
    std::vector<ParseState> state_stack;
    std::vector<uint32_t> node_stack;
    
    // Current parsing state
    ParseState current_state;
    uint32_t current_node_index;
    
    // Error handling with detailed position info
    bool has_error;
    std::string error_message;
    size_t error_token_index;
    size_t error_line;
    size_t error_column;
    
    ParseContext(const std::vector<Token>& tokens)
        : tokens(tokens), current_token(0), token_count(tokens.size()),
          current_state(ParseState::PROGRAM), current_node_index(0),
          has_error(false), error_token_index(0), error_line(0), error_column(0) {
        // Pre-allocate aggressively for typical program size
        nodes.reserve(DEFAULT_NODES_CAPACITY);  // Increased from 1024
        state_stack.reserve(DEFAULT_STACK_CAPACITY);  // Increased from 64
        node_stack.reserve(DEFAULT_STACK_CAPACITY);   // Increased from 64
    }
};

// RAII helper for expression stack management
class ExpressionStackGuard {
private:
    std::vector<uint32_t>& stack;
    size_t initial_size;
    
public:
    ExpressionStackGuard(std::vector<uint32_t>& s) : stack(s), initial_size(s.size()) {}
    
    ~ExpressionStackGuard() {
        // Restore stack to initial size if needed
        if (stack.size() > initial_size) {
            stack.resize(initial_size);
        }
    }
    
    // Get the result node (last item on stack)
    uint32_t get_result() const {
        return stack.empty() ? 0 : stack.back();
    }
    
    // Pop the result and return it
    uint32_t pop_result() {
        if (stack.empty()) return 0;
        uint32_t result = stack.back();
        stack.pop_back();
        return result;
    }
};

// High-performance parser class
class Parser {
private:
    ParseContext ctx;
    
    // Operator precedence lookup
    static const OperatorInfo OPERATOR_TABLE[];
    static const size_t OPERATOR_TABLE_SIZE;
    
    // Fast token access
    const Token& current_token() const;
    const Token& peek_token(size_t offset = 1) const;
    bool at_end() const;
    void advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    
    // Node management
    uint32_t create_node(NodeType type);
    void add_child(uint32_t parent_index, uint32_t child_index);
    void set_parent(uint32_t child_index, uint32_t parent_index);
    
    // Error handling - fail fast with detailed position info
    void error(const std::string& message);
    void error_at_current(const std::string& message);
    void error_at_token(size_t token_index, const std::string& message);
    void synchronize();  // Error recovery - skip to next statement boundary
    
    // State stack management for iterative parsing
    void push_state(ParseState state, uint32_t node_index = 0);
    void pop_state();
    void clear_state_stack();
    bool is_state_stack_safe() const;  // Check for stack overflow
    
    // Matrix literal validation utilities
    bool validate_matrix_dimensions(const std::vector<std::vector<uint32_t>>& rows) const;
    void parse_matrix_elements(std::vector<std::vector<uint32_t>>& matrix_rows);
    
    // Parsing methods - iterative state machine
    void parse_program();
    void parse_statement();
    void parse_expression();
    void parse_binary_expression(uint8_t min_precedence);
    void parse_unary_expression();
    void parse_primary();
    void parse_postfix_expressions();
    void parse_matrix_literal();
    void parse_function_call();
    void parse_block();
    void parse_if_statement();
    void parse_while_statement();
    void parse_for_statement();
    void parse_function_definition();
    void parse_assignment();
    
    // Utility methods
    BinaryOpType token_to_binary_op(TokenType token_type) const;
    UnaryOpType token_to_unary_op(TokenType token_type) const;
    uint8_t get_precedence(TokenType token_type) const;
    bool is_right_associative(TokenType token_type) const;
    
    // Engineering-specific optimizations
    void optimize_matrix_operations();
    void optimize_constant_folding();
    
public:
    Parser(const std::vector<Token>& tokens);
    
    // Main parsing entry point
    uint32_t parse();
    
    // Access parsed AST
    const std::vector<ASTNode>& get_nodes() const { return ctx.nodes; }
    const StringTable& get_strings() const { return ctx.strings; }
    
    // Error information with detailed position tracking
    bool has_error() const { return ctx.has_error; }
    const std::string& get_error() const { return ctx.error_message; }
    size_t get_error_token_index() const { return ctx.error_token_index; }
    size_t get_error_line() const { return ctx.error_line; }
    size_t get_error_column() const { return ctx.error_column; }
    
    // Memory usage statistics and optimization
    size_t get_memory_usage() const;
    void optimize_memory_usage();  // Shrink vectors to fit after parsing
    void create_program_block();   // Create proper block structure for program
    size_t get_nodes_capacity() const { return ctx.nodes.capacity(); }
    size_t get_strings_memory() const { return ctx.strings.memory_usage(); }
    
    // AST traversal utilities
    void print_ast(uint32_t node_index = 1, int indent = 0) const;
    std::vector<uint32_t> get_children(uint32_t node_index) const;
    uint32_t get_parent(uint32_t node_index) const;
    
    // Engineering-specific query methods
    bool is_matrix_operation(uint32_t node_index) const;
    bool is_constant_expression(uint32_t node_index) const;
    std::vector<uint32_t> find_matrix_multiplications() const;
    std::vector<uint32_t> find_function_calls() const;
    
    // Matrix literal validation methods
    bool is_empty_matrix_valid() const;
    bool validate_matrix_rows(const std::vector<std::vector<uint32_t>>& rows) const;
    std::string get_matrix_dimension_error(size_t row_index, size_t expected_cols, size_t actual_cols) const;
};

// Utility functions for AST analysis
namespace AST {
    // Fast node type checking
    inline bool is_literal(const ASTNode& node) {
        return node.type >= NodeType::INTEGER_LITERAL && 
               node.type <= NodeType::BOOLEAN_LITERAL;
    }
    
    inline bool is_binary_op(const ASTNode& node) {
        return node.type == NodeType::BINARY_OP;
    }
    
    inline bool is_matrix_op(const ASTNode& node) {
        return node.type == NodeType::MATRIX_LITERAL ||
               node.type == NodeType::MATRIX_MULTIPLY ||
               node.type == NodeType::MATRIX_ACCESS;
    }
    
    // Fast matrix operation validation
    inline bool is_valid_matrix_literal(const ASTNode& node) {
        return node.type == NodeType::MATRIX_LITERAL && 
               node.matrix_literal.validation_error == MatrixError::NONE;
    }
    
    inline bool is_empty_matrix(const ASTNode& node) {
        return node.type == NodeType::MATRIX_LITERAL && 
               node.matrix_literal.is_empty;
    }
    
    // Matrix dimension validation
    inline bool has_valid_dimensions(const ASTNode& node) {
        return node.type == NodeType::MATRIX_LITERAL &&
               node.matrix_literal.rows > 0 && node.matrix_literal.cols > 0 &&
               node.matrix_literal.rows <= MAX_MATRIX_DIMENSIONS &&
               node.matrix_literal.cols <= MAX_MATRIX_DIMENSIONS;
    }
    
    // State stack safety check
    inline bool is_parse_stack_safe(size_t current_depth) {
        return current_depth < MAX_STATE_STACK_DEPTH;
    }
    
    // Fast precedence checking
    inline bool has_higher_precedence(BinaryOpType op1, BinaryOpType op2) {
        // Implementation depends on operator precedence rules
        return static_cast<uint8_t>(op1) > static_cast<uint8_t>(op2);
    }
    
    // Type-safe node accessor helpers
    template<typename T>
    T& get_node_data(ASTNode& node);
    
    template<typename T>
    const T& get_node_data(const ASTNode& node);
}

} // namespace Dakota

#endif // PARSER_H
