#include "parser.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace Dakota {

// Constants for better type safety and clarity
constexpr uint32_t INVALID_INDEX = UINT32_MAX;
constexpr uint32_t ROOT_NODE_INDEX = 0;

// Operator precedence table - ordered by precedence (higher = higher precedence)
const OperatorInfo Parser::OPERATOR_TABLE[] = {
    {BinaryOpType::OR,     1, false},    // or
    {BinaryOpType::AND,    2, false},    // and
    {BinaryOpType::EQ,     3, false},    // ==
    {BinaryOpType::NE,     3, false},    // !=
    {BinaryOpType::LT,     4, false},    // <
    {BinaryOpType::LE,     4, false},    // <=
    {BinaryOpType::GT,     4, false},    // >
    {BinaryOpType::GE,     4, false},    // >=
    {BinaryOpType::ADD,    5, false},    // +
    {BinaryOpType::SUB,    5, false},    // -
    {BinaryOpType::MUL,    6, false},    // *
    {BinaryOpType::DIV,    6, false},    // /
    {BinaryOpType::MATMUL, 6, false},    // mult
    {BinaryOpType::POW,    7, true},     // ** (right associative)
};

const size_t Parser::OPERATOR_TABLE_SIZE = sizeof(OPERATOR_TABLE) / sizeof(OperatorInfo);

// StringTable implementation
uint32_t StringTable::add_string(std::string_view str) {
    if (str.empty()) return 0;
    
    uint32_t index = static_cast<uint32_t>(offsets.size());
    offsets.push_back(static_cast<uint32_t>(data.size()));
    
    // Add string data
    data.insert(data.end(), str.begin(), str.end());
    data.push_back('\0'); // Null terminator
    
    return index;
}

std::string_view StringTable::get_string(uint32_t index) const {
    if (index >= offsets.size()) return "";
    
    uint32_t start = offsets[index];
    uint32_t end = (index + 1 < offsets.size()) ? offsets[index + 1] : static_cast<uint32_t>(data.size());
    
    // Exclude null terminator from view
    return std::string_view(data.data() + start, end - start - 1);
}

const char* StringTable::get_c_string(uint32_t index) const {
    if (index >= offsets.size()) return "";
    return data.data() + offsets[index];
}

void StringTable::clear() {
    data.clear();
    offsets.clear();
}

// Parser implementation
Parser::Parser(const std::vector<Token>& tokens) : ctx(tokens) {
    // Add empty string at index 0
    ctx.strings.add_string("");
    
    // Create root program node
    ctx.nodes.emplace_back(NodeType::PROGRAM, 0);
    ctx.current_node_index = ROOT_NODE_INDEX;
}

const Token& Parser::current_token() const {
    if (ctx.current_token >= ctx.token_count) {
        static const Token eof_token(TokenType::EOF_TOKEN, "", 0, 0);
        return eof_token;
    }
    return ctx.tokens[ctx.current_token];
}

const Token& Parser::peek_token(size_t offset) const {
    size_t index = ctx.current_token + offset;
    if (index >= ctx.token_count) {
        static const Token eof_token(TokenType::EOF_TOKEN, "", 0, 0);
        return eof_token;
    }
    return ctx.tokens[index];
}

bool Parser::at_end() const {
    return ctx.current_token >= ctx.token_count || 
           current_token().type == TokenType::EOF_TOKEN;
}

void Parser::advance() {
    if (!at_end()) {
        ctx.current_token++;
    }
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return current_token().type == type;
}

uint32_t Parser::create_node(NodeType type) {
    uint32_t index = static_cast<uint32_t>(ctx.nodes.size());
    ctx.nodes.emplace_back(type, ctx.current_token);
    return index;
}

void Parser::add_child(uint32_t parent_index, uint32_t child_index) {
    if (parent_index >= ctx.nodes.size() || child_index >= ctx.nodes.size()) {
        return;
    }
    
    ASTNode& parent = ctx.nodes[parent_index];
    ASTNode& child = ctx.nodes[child_index];
    
    child.parent_index = parent_index;
    
    if (parent.first_child_index == INVALID_INDEX) {
        parent.first_child_index = child_index;
    } else {
        // Find last sibling and add new child
        uint32_t sibling_index = parent.first_child_index;
        while (ctx.nodes[sibling_index].next_sibling_index != INVALID_INDEX) {
            sibling_index = ctx.nodes[sibling_index].next_sibling_index;
        }
        ctx.nodes[sibling_index].next_sibling_index = child_index;
    }
}

void Parser::set_parent(uint32_t child_index, uint32_t parent_index) {
    if (child_index < ctx.nodes.size()) {
        ctx.nodes[child_index].parent_index = parent_index;
    }
}

void Parser::error(const std::string& message) {
    ctx.has_error = true;
    ctx.error_message = message;
    ctx.error_token_index = ctx.current_token;
}

void Parser::error_at_current(const std::string& message) {
    const Token& token = current_token();
    error("Line " + std::to_string(token.line) + ", Column " + 
          std::to_string(token.column) + ": " + message);
}

void Parser::synchronize() {
    // Skip tokens until we find a statement boundary
    while (!at_end()) {
        if (current_token().type == TokenType::NEWLINE) {
            advance();
            return;
        }
        
        // Look for statement keywords
        TokenType type = current_token().type;
        if (type == TokenType::IF || type == TokenType::WHILE || 
            type == TokenType::FOR || type == TokenType::FUNCTION ||
            type == TokenType::RETURN) {
            return;
        }
        
        advance();
    }
}

void Parser::parse_postfix_expressions() {
    while (true) {
        if (check(TokenType::LBRACKET)) {
            // Array access
            advance(); // consume '['
            
            uint32_t object_node = ctx.node_stack.back();
            ctx.node_stack.pop_back();
            
            parse_expression();
            uint32_t index_node = ctx.node_stack.back();
            ctx.node_stack.pop_back();
            
            if (!match(TokenType::RBRACKET)) {
                error_at_current("Expected ']' after array index");
            }
            
            uint32_t access_node = create_node(NodeType::ARRAY_ACCESS);
            ctx.nodes[access_node].array_access.object_index = object_node;
            ctx.nodes[access_node].array_access.index_index = index_node;
            
            ctx.node_stack.push_back(access_node);
            
        } else if (check(TokenType::DOT)) {
            // Member access
            advance(); // consume '.'
            
            if (!check(TokenType::IDENTIFIER)) {
                error_at_current("Expected member name after '.'");
                break;
            }
            
            uint32_t object_node = ctx.node_stack.back();
            ctx.node_stack.pop_back();
            
            std::string_view member_name = current_token().value;
            advance();
            
            uint32_t member_node = create_node(NodeType::MEMBER_ACCESS);
            ctx.nodes[member_node].member_access.object_index = object_node;
            ctx.nodes[member_node].member_access.member_name_index = ctx.strings.add_string(member_name);
            
            ctx.node_stack.push_back(member_node);
            
        } else {
            break; // No more postfix operations
        }
    }
}

// Main parsing entry point
uint32_t Parser::parse() {
    try {
        parse_program();
        return 0; // Root node index
    } catch (const std::exception& e) {
        error(std::string("Parse error: ") + e.what());
        return 0;
    }
}

void Parser::parse_program() {
    uint32_t program_index = ROOT_NODE_INDEX; // Root node
    
    while (!at_end()) {
        if (match(TokenType::NEWLINE)) {
            continue; // Skip empty lines
        }
        
        size_t start_token = ctx.current_token;
        
        try {
            parse_statement();
        } catch (const std::exception& e) {
            error_at_current(std::string("Statement parse error: ") + e.what());
            synchronize(); // Try to recover
            continue;
        }
        
        // Fail fast: if we didn't advance, we're stuck
        if (ctx.current_token == start_token && !at_end()) {
            error_at_current("Unexpected token in program");
            synchronize(); // Try to recover
            continue;
        }
        
        // Continue parsing even with errors for better error reporting
        if (ctx.has_error) {
            ctx.has_error = false; // Reset error flag for next statement
            synchronize(); // Try to recover
        }
    }
}

void Parser::parse_statement() {
    // Skip newlines
    while (match(TokenType::NEWLINE)) {}
    
    if (at_end()) return;
    
    // Function definition
    if (check(TokenType::FUNCTION)) {
        parse_function_definition();
        return;
    }
    
    // Control flow
    if (check(TokenType::IF)) {
        parse_if_statement();
        return;
    }
    
    if (check(TokenType::WHILE)) {
        parse_while_statement();
        return;
    }
    
    if (check(TokenType::FOR)) {
        parse_for_statement();
        return;
    }
    
    if (check(TokenType::RETURN)) {
        advance(); // consume 'return'
        uint32_t return_node = create_node(NodeType::RETURN_STATEMENT);
        
        if (!check(TokenType::NEWLINE) && !at_end()) {
            ctx.node_stack.push_back(return_node);
            parse_expression();
            uint32_t expr_node = ctx.node_stack.back();
            ctx.node_stack.pop_back();
            ctx.nodes[return_node].return_statement.value_index = expr_node;
        } else {
            ctx.nodes[return_node].return_statement.value_index = 0; // void return
        }
        
        add_child(ROOT_NODE_INDEX, return_node); // Add to program
        return;
    }
    
    // Assignment or expression statement
    if (check(TokenType::IDENTIFIER)) {
        // Look ahead to see if this is an assignment
        if (peek_token().type == TokenType::ASSIGN) {
            parse_assignment();
            return;
        }
    }
    
    // Expression statement
    uint32_t expr_stmt_node = create_node(NodeType::EXPRESSION_STATEMENT);
    ctx.node_stack.push_back(expr_stmt_node);
    parse_expression();
    
    if (!ctx.node_stack.empty()) {
        uint32_t expr_node = ctx.node_stack.back();
        ctx.node_stack.pop_back();
        ctx.nodes[expr_stmt_node].expression_statement.expression_index = expr_node;
        add_child(ROOT_NODE_INDEX, expr_stmt_node);
    }
}

void Parser::parse_expression() {
    parse_binary_expression(0); // Start with lowest precedence
}

void Parser::parse_binary_expression(uint8_t min_precedence) {
    parse_unary_expression();
    
    while (!at_end()) {
        TokenType op_token = current_token().type;
        uint8_t precedence = get_precedence(op_token);
        
        if (precedence < min_precedence) {
            break;
        }
        
        bool right_assoc = is_right_associative(op_token);
        uint8_t next_min_prec = right_assoc ? precedence : precedence + 1;
        
        advance(); // consume operator
        
        uint32_t left_node = ctx.node_stack.back();
        ctx.node_stack.pop_back();
        
        parse_binary_expression(next_min_prec);
        
        uint32_t right_node = ctx.node_stack.back();
        ctx.node_stack.pop_back();
        
        uint32_t binary_node = create_node(NodeType::BINARY_OP);
        ctx.nodes[binary_node].binary_op.op_type = token_to_binary_op(op_token);
        ctx.nodes[binary_node].binary_op.left_index = left_node;
        ctx.nodes[binary_node].binary_op.right_index = right_node;
        
        ctx.node_stack.push_back(binary_node);
    }
}

void Parser::parse_unary_expression() {
    if (check(TokenType::MINUS) || check(TokenType::NOT)) {
        TokenType op_token = current_token().type;
        advance();
        
        parse_unary_expression();
        
        uint32_t operand_node = ctx.node_stack.back();
        ctx.node_stack.pop_back();
        
        uint32_t unary_node = create_node(NodeType::UNARY_OP);
        ctx.nodes[unary_node].unary_op.op_type = token_to_unary_op(op_token);
        ctx.nodes[unary_node].unary_op.operand_index = operand_node;
        
        ctx.node_stack.push_back(unary_node);
    } else {
        parse_primary();
    }
}

void Parser::parse_primary() {
    // Integer literal
    if (check(TokenType::INTEGER)) {
        uint32_t node = create_node(NodeType::INTEGER_LITERAL);
        ctx.nodes[node].integer_literal.value = std::stoll(current_token().value);
        advance();
        ctx.node_stack.push_back(node);
        return;
    }
    
    // Float literal
    if (check(TokenType::FLOAT)) {
        uint32_t node = create_node(NodeType::FLOAT_LITERAL);
        ctx.nodes[node].float_literal.value = std::stod(current_token().value);
        advance();
        ctx.node_stack.push_back(node);
        return;
    }
    
    // String literal
    if (check(TokenType::STRING)) {
        uint32_t node = create_node(NodeType::STRING_LITERAL);
        uint32_t string_index = ctx.strings.add_string(current_token().value);
        ctx.nodes[node].string_literal.string_index = string_index;
        ctx.nodes[node].string_literal.length = static_cast<uint32_t>(current_token().value.length());
        advance();
        ctx.node_stack.push_back(node);
        return;
    }
    
    // Boolean literal
    if (check(TokenType::TRUE) || check(TokenType::FALSE)) {
        uint32_t node = create_node(NodeType::BOOLEAN_LITERAL);
        ctx.nodes[node].boolean_literal.value = check(TokenType::TRUE);
        advance();
        ctx.node_stack.push_back(node);
        return;
    }
    
    // Matrix literal
    if (check(TokenType::LBRACKET)) {
        parse_matrix_literal();
        return;
    }
    
    // Parenthesized expression
    if (check(TokenType::LPAREN)) {
        advance(); // consume '('
        parse_expression();
        if (!match(TokenType::RPAREN)) {
            error_at_current("Expected ')' after expression");
        }
        return;
    }
    
    // Identifier (variable or function call)
    if (check(TokenType::IDENTIFIER)) {
        std::string_view name = current_token().value;
        advance();
        
        // Check for function call
        if (check(TokenType::LPAREN)) {
            // Function call
            uint32_t func_node = create_node(NodeType::FUNCTION_CALL);
            ctx.nodes[func_node].function_call.name_index = ctx.strings.add_string(name);
            
            advance(); // consume '('
            
            std::vector<uint32_t> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    parse_expression();
                    args.push_back(ctx.node_stack.back());
                    ctx.node_stack.pop_back();
                } while (match(TokenType::COMMA));
            }
            
            if (!match(TokenType::RPAREN)) {
                error_at_current("Expected ')' after function arguments");
            }
            
            ctx.nodes[func_node].function_call.arg_count = static_cast<uint32_t>(args.size());
            if (!args.empty()) {
                ctx.nodes[func_node].function_call.args_start_index = args[0];
                // Link arguments as siblings
                for (size_t i = 1; i < args.size(); i++) {
                    ctx.nodes[args[i-1]].next_sibling_index = args[i];
                }
            }
            
            ctx.node_stack.push_back(func_node);
            
            // Check for postfix operations (array access, member access)
            parse_postfix_expressions();
            
        } else {
            // Variable reference
            uint32_t id_node = create_node(NodeType::IDENTIFIER);
            ctx.nodes[id_node].identifier.name_index = ctx.strings.add_string(name);
            ctx.node_stack.push_back(id_node);
            
            // Check for postfix operations (array access, member access)
            parse_postfix_expressions();
        }
        return;
    }
    
    error_at_current("Expected expression");
}

void Parser::parse_matrix_literal() {
    advance(); // consume '['
    
    uint32_t matrix_node = create_node(NodeType::MATRIX_LITERAL);
    std::vector<uint32_t> elements;
    uint32_t rows = 0, cols = 0;
    
    if (!check(TokenType::RBRACKET)) {
        // Parse first row
        uint32_t row_cols = 0;
        do {
            parse_expression();
            elements.push_back(ctx.node_stack.back());
            ctx.node_stack.pop_back();
            row_cols++;
        } while (match(TokenType::COMMA));
        
        cols = row_cols;
        rows = 1;
        
        // Parse additional rows
        while (match(TokenType::SEMICOLON)) {
            row_cols = 0;
            do {
                parse_expression();
                elements.push_back(ctx.node_stack.back());
                ctx.node_stack.pop_back();
                row_cols++;
            } while (match(TokenType::COMMA));
            
            if (cols != row_cols) {
                error_at_current("Inconsistent matrix row lengths");
                return;
            }
            rows++;
        }
    }
    
    if (!match(TokenType::RBRACKET)) {
        error_at_current("Expected ']' after matrix literal");
        return;
    }
    
    ctx.nodes[matrix_node].matrix_literal.rows = rows;
    ctx.nodes[matrix_node].matrix_literal.cols = cols;
    
    if (!elements.empty()) {
        ctx.nodes[matrix_node].matrix_literal.elements_start_index = elements[0];
        // Link elements as siblings
        for (size_t i = 1; i < elements.size(); i++) {
            ctx.nodes[elements[i-1]].next_sibling_index = elements[i];
        }
    }
    
    ctx.node_stack.push_back(matrix_node);
}

void Parser::parse_assignment() {
    if (!check(TokenType::IDENTIFIER)) {
        error_at_current("Expected identifier in assignment");
        return;
    }
    
    std::string_view name = current_token().value;
    advance();
    
    if (!match(TokenType::ASSIGN)) {
        error_at_current("Expected '=' in assignment");
        return;
    }
    
    uint32_t assign_node = create_node(NodeType::ASSIGNMENT);
    
    // Create target identifier
    uint32_t target_node = create_node(NodeType::IDENTIFIER);
    ctx.nodes[target_node].identifier.name_index = ctx.strings.add_string(name);
    
    // Parse value expression
    parse_expression();
    uint32_t value_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    ctx.nodes[assign_node].assignment.target_index = target_node;
    ctx.nodes[assign_node].assignment.value_index = value_node;
    
    add_child(ROOT_NODE_INDEX, assign_node); // Add to program
}

void Parser::parse_if_statement() {
    advance(); // consume 'if'
    
    uint32_t if_node = create_node(NodeType::IF_STATEMENT);
    
    // Parse condition
    parse_expression();
    uint32_t condition_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    if (!match(TokenType::COLON)) {
        error_at_current("Expected ':' after if condition");
    }
    
    // Parse then block
    parse_block();
    uint32_t then_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    uint32_t else_node = 0;
    if (match(TokenType::ELSE)) {
        if (match(TokenType::COLON)) {
            parse_block();
            else_node = ctx.node_stack.back();
            ctx.node_stack.pop_back();
        } else {
            error_at_current("Expected ':' after else");
        }
    }
    
    ctx.nodes[if_node].if_statement.condition_index = condition_node;
    ctx.nodes[if_node].if_statement.then_block_index = then_node;
    ctx.nodes[if_node].if_statement.else_block_index = else_node;
    
    add_child(ROOT_NODE_INDEX, if_node);
}

void Parser::parse_while_statement() {
    advance(); // consume 'while'
    
    uint32_t while_node = create_node(NodeType::WHILE_STATEMENT);
    
    // Parse condition
    parse_expression();
    uint32_t condition_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    if (!match(TokenType::COLON)) {
        error_at_current("Expected ':' after while condition");
    }
    
    // Parse body
    parse_block();
    uint32_t body_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    ctx.nodes[while_node].while_statement.condition_index = condition_node;
    ctx.nodes[while_node].while_statement.body_index = body_node;
    
    add_child(ROOT_NODE_INDEX, while_node);
}

void Parser::parse_for_statement() {
    advance(); // consume 'for'
    
    uint32_t for_node = create_node(NodeType::FOR_STATEMENT);
    
    // Parse loop variable
    if (!check(TokenType::IDENTIFIER)) {
        error_at_current("Expected loop variable in for statement");
        return;
    }
    
    std::string_view var_name = current_token().value;
    advance();
    
    if (!match(TokenType::IN)) {
        error_at_current("Expected 'in' after for loop variable");
        return;
    }
    
    // Create loop variable identifier
    uint32_t var_node = create_node(NodeType::IDENTIFIER);
    ctx.nodes[var_node].identifier.name_index = ctx.strings.add_string(var_name);
    
    // Parse iterable expression
    parse_expression();
    uint32_t iterable_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    if (!match(TokenType::COLON)) {
        error_at_current("Expected ':' after for loop iterable");
        return;
    }
    
    // Parse body
    parse_block();
    uint32_t body_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    ctx.nodes[for_node].for_statement.variable_index = var_node;
    ctx.nodes[for_node].for_statement.iterable_index = iterable_node;
    ctx.nodes[for_node].for_statement.body_index = body_node;
    
    add_child(ROOT_NODE_INDEX, for_node);
}

void Parser::parse_function_definition() {
    advance(); // consume 'function'
    
    if (!check(TokenType::IDENTIFIER)) {
        error_at_current("Expected function name");
        return;
    }
    
    std::string_view name = current_token().value;
    advance();
    
    uint32_t func_node = create_node(NodeType::FUNCTION_DEF);
    ctx.nodes[func_node].function_def.name_index = ctx.strings.add_string(name);
    
    if (!match(TokenType::LPAREN)) {
        error_at_current("Expected '(' after function name");
    }
    
    // Parse parameters
    std::vector<uint32_t> params;
    if (!check(TokenType::RPAREN)) {
        do {
            if (!check(TokenType::IDENTIFIER)) {
                error_at_current("Expected parameter name");
                break;
            }
            
            uint32_t param_node = create_node(NodeType::IDENTIFIER);
            ctx.nodes[param_node].identifier.name_index = ctx.strings.add_string(current_token().value);
            params.push_back(param_node);
            advance();
        } while (match(TokenType::COMMA));
    }
    
    if (!match(TokenType::RPAREN)) {
        error_at_current("Expected ')' after parameters");
    }
    
    if (!match(TokenType::COLON)) {
        error_at_current("Expected ':' after function signature");
    }
    
    // Parse body
    parse_block();
    uint32_t body_node = ctx.node_stack.back();
    ctx.node_stack.pop_back();
    
    ctx.nodes[func_node].function_def.param_count = static_cast<uint32_t>(params.size());
    ctx.nodes[func_node].function_def.body_index = body_node;
    
    if (!params.empty()) {
        ctx.nodes[func_node].function_def.params_start_index = params[0];
        // Link parameters as siblings
        for (size_t i = 1; i < params.size(); i++) {
            ctx.nodes[params[i-1]].next_sibling_index = params[i];
        }
    }
    
    add_child(ROOT_NODE_INDEX, func_node);
}

void Parser::parse_block() {
    if (!match(TokenType::NEWLINE)) {
        error_at_current("Expected newline after ':'");
    }
    
    if (!match(TokenType::INDENT)) {
        error_at_current("Expected indentation for block");
    }
    
    uint32_t block_node = create_node(NodeType::BLOCK);
    std::vector<uint32_t> statements;
    
    while (!check(TokenType::DEDENT) && !at_end()) {
        if (match(TokenType::NEWLINE)) {
            continue;
        }
        
        size_t stmt_start = ctx.nodes.size();
        parse_statement();
        
        // Add any new statements to the block
        for (size_t i = stmt_start; i < ctx.nodes.size(); i++) {
            if (ctx.nodes[i].parent_index == ROOT_NODE_INDEX) { // Top-level statement
                statements.push_back(static_cast<uint32_t>(i));
                ctx.nodes[i].parent_index = block_node;
            }
        }
    }
    
    if (!match(TokenType::DEDENT)) {
        error_at_current("Expected dedentation after block");
    }
    
    ctx.nodes[block_node].block.statement_count = static_cast<uint32_t>(statements.size());
    if (!statements.empty()) {
        ctx.nodes[block_node].block.statements_start_index = statements[0];
        // Link statements as siblings
        for (size_t i = 1; i < statements.size(); i++) {
            ctx.nodes[statements[i-1]].next_sibling_index = statements[i];
        }
    }
    
    ctx.node_stack.push_back(block_node);
}

// Utility methods
BinaryOpType Parser::token_to_binary_op(TokenType token_type) const {
    switch (token_type) {
        case TokenType::PLUS: return BinaryOpType::ADD;
        case TokenType::MINUS: return BinaryOpType::SUB;
        case TokenType::MULTIPLY: return BinaryOpType::MUL;
        case TokenType::DIVIDE: return BinaryOpType::DIV;
        case TokenType::POWER: return BinaryOpType::POW;
        case TokenType::MATMUL: return BinaryOpType::MATMUL;
        case TokenType::EQUAL: return BinaryOpType::EQ;
        case TokenType::NOT_EQUAL: return BinaryOpType::NE;
        case TokenType::LESS: return BinaryOpType::LT;
        case TokenType::LESS_EQUAL: return BinaryOpType::LE;
        case TokenType::GREATER: return BinaryOpType::GT;
        case TokenType::GREATER_EQUAL: return BinaryOpType::GE;
        case TokenType::AND: return BinaryOpType::AND;
        case TokenType::OR: return BinaryOpType::OR;
        default: return BinaryOpType::ADD; // Default
    }
}

UnaryOpType Parser::token_to_unary_op(TokenType token_type) const {
    switch (token_type) {
        case TokenType::MINUS: return UnaryOpType::NEGATE;
        case TokenType::NOT: return UnaryOpType::NOT;
        default: return UnaryOpType::NEGATE; // Default
    }
}

uint8_t Parser::get_precedence(TokenType token_type) const {
    BinaryOpType op_type = token_to_binary_op(token_type);
    
    for (size_t i = 0; i < OPERATOR_TABLE_SIZE; i++) {
        if (OPERATOR_TABLE[i].op_type == op_type) {
            return OPERATOR_TABLE[i].precedence;
        }
    }
    
    return 0; // Lowest precedence
}

bool Parser::is_right_associative(TokenType token_type) const {
    BinaryOpType op_type = token_to_binary_op(token_type);
    
    for (size_t i = 0; i < OPERATOR_TABLE_SIZE; i++) {
        if (OPERATOR_TABLE[i].op_type == op_type) {
            return OPERATOR_TABLE[i].right_associative;
        }
    }
    
    return false;
}

size_t Parser::get_memory_usage() const {
    return ctx.nodes.size() * sizeof(ASTNode) + ctx.strings.memory_usage();
}

void Parser::create_program_block() {
    // Create a block node to hold all top-level statements
    uint32_t block_node = create_node(NodeType::BLOCK);
    
    // Find all direct children of the program node
    std::vector<uint32_t> top_level_statements;
    for (uint32_t i = 1; i < ctx.nodes.size(); i++) {
        if (ctx.nodes[i].parent_index == ROOT_NODE_INDEX) {
            top_level_statements.push_back(i);
        }
    }
    
    // Re-parent all top-level statements to the block
    for (uint32_t stmt_index : top_level_statements) {
        ctx.nodes[stmt_index].parent_index = block_node;
    }
    
    // Set up the block's statement list
    ctx.nodes[block_node].block.statement_count = static_cast<uint32_t>(top_level_statements.size());
    if (!top_level_statements.empty()) {
        ctx.nodes[block_node].block.statements_start_index = top_level_statements[0];
        // Link statements as siblings
        for (size_t i = 1; i < top_level_statements.size(); i++) {
            ctx.nodes[top_level_statements[i-1]].next_sibling_index = top_level_statements[i];
        }
    }
    
    // Make the block the only child of the program
    ctx.nodes[ROOT_NODE_INDEX].first_child_index = block_node;
    ctx.nodes[block_node].parent_index = ROOT_NODE_INDEX;
}

void Parser::print_ast(uint32_t node_index, int indent) const {
    if (node_index >= ctx.nodes.size()) return;
    
    const ASTNode& node = ctx.nodes[node_index];
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
    
    // Print node type
    switch (node.type) {
        case NodeType::PROGRAM:
            std::cout << "PROGRAM\n";
            break;
        case NodeType::INTEGER_LITERAL:
            std::cout << "INTEGER: " << node.integer_literal.value << "\n";
            break;
        case NodeType::FLOAT_LITERAL:
            std::cout << "FLOAT: " << node.float_literal.value << "\n";
            break;
        case NodeType::STRING_LITERAL:
            std::cout << "STRING: \"" << ctx.strings.get_string(node.string_literal.string_index) << "\"\n";
            break;
        case NodeType::BOOLEAN_LITERAL:
            std::cout << "BOOLEAN: " << (node.boolean_literal.value ? "true" : "false") << "\n";
            break;
        case NodeType::IDENTIFIER:
            std::cout << "IDENTIFIER: " << ctx.strings.get_string(node.identifier.name_index) << "\n";
            break;
        case NodeType::BINARY_OP:
            std::cout << "BINARY_OP: " << static_cast<int>(node.binary_op.op_type) << "\n";
            print_ast(node.binary_op.left_index, indent + 1);
            print_ast(node.binary_op.right_index, indent + 1);
            break;
        case NodeType::ASSIGNMENT:
            std::cout << "ASSIGNMENT\n";
            print_ast(node.assignment.target_index, indent + 1);
            print_ast(node.assignment.value_index, indent + 1);
            break;
        case NodeType::MATRIX_LITERAL:
            std::cout << "MATRIX: " << node.matrix_literal.rows << "x" << node.matrix_literal.cols << "\n";
            break;
        case NodeType::ARRAY_ACCESS:
            std::cout << "ARRAY_ACCESS\n";
            print_ast(node.array_access.object_index, indent + 1);
            print_ast(node.array_access.index_index, indent + 1);
            break;
        case NodeType::MEMBER_ACCESS:
            std::cout << "MEMBER_ACCESS: " << ctx.strings.get_string(node.member_access.member_name_index) << "\n";
            print_ast(node.member_access.object_index, indent + 1);
            break;
        default:
            std::cout << "NODE_TYPE: " << static_cast<int>(node.type) << "\n";
            break;
    }
    
    // Print children
    uint32_t child_index = node.first_child_index;
    while (child_index != 0) {
        print_ast(child_index, indent + 1);
        child_index = ctx.nodes[child_index].next_sibling_index;
    }
}

std::vector<uint32_t> Parser::get_children(uint32_t node_index) const {
    std::vector<uint32_t> children;
    if (node_index >= ctx.nodes.size()) return children;
    
    uint32_t child_index = ctx.nodes[node_index].first_child_index;
    while (child_index != 0 && child_index < ctx.nodes.size()) {
        children.push_back(child_index);
        child_index = ctx.nodes[child_index].next_sibling_index;
    }
    
    return children;
}

bool Parser::is_matrix_operation(uint32_t node_index) const {
    if (node_index >= ctx.nodes.size()) return false;
    return AST::is_matrix_op(ctx.nodes[node_index]);
}

std::vector<uint32_t> Parser::find_matrix_multiplications() const {
    std::vector<uint32_t> result;
    
    for (uint32_t i = 0; i < ctx.nodes.size(); i++) {
        const ASTNode& node = ctx.nodes[i];
        if (node.type == NodeType::BINARY_OP && 
            node.binary_op.op_type == BinaryOpType::MATMUL) {
            result.push_back(i);
        }
    }
    
    return result;
}

std::vector<uint32_t> Parser::find_function_calls() const {
    std::vector<uint32_t> result;
    
    for (uint32_t i = 0; i < ctx.nodes.size(); i++) {
        const ASTNode& node = ctx.nodes[i];
        if (node.type == NodeType::FUNCTION_CALL) {
            result.push_back(i);
        }
    }
    
    return result;
}

} // namespace Dakota
