#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

namespace Dakota {

// Value types that can be stored and manipulated
class Value {
public:
    enum class Type {
        INTEGER,
        FLOAT,
        STRING,
        BOOLEAN,
        MATRIX,
        NONE
    };

private:
    Type type_;
    std::variant<int64_t, double, std::string, bool, std::vector<std::vector<double>>> value_;

public:
    // Constructors
    Value() : type_(Type::NONE), value_(0) {}
    Value(int64_t val) : type_(Type::INTEGER), value_(val) {}
    Value(double val) : type_(Type::FLOAT), value_(val) {}
    Value(const std::string& val) : type_(Type::STRING), value_(val) {}
    Value(bool val) : type_(Type::BOOLEAN), value_(val) {}
    Value(const std::vector<std::vector<double>>& val) : type_(Type::MATRIX), value_(val) {}

    // Type checking
    Type get_type() const { return type_; }
    bool is_integer() const { return type_ == Type::INTEGER; }
    bool is_float() const { return type_ == Type::FLOAT; }
    bool is_string() const { return type_ == Type::STRING; }
    bool is_boolean() const { return type_ == Type::BOOLEAN; }
    bool is_matrix() const { return type_ == Type::MATRIX; }
    bool is_none() const { return type_ == Type::NONE; }
    bool is_numeric() const { return is_integer() || is_float(); }

    // Value accessors
    int64_t as_integer() const;
    double as_float() const;
    const std::string& as_string() const;
    bool as_boolean() const;
    const std::vector<std::vector<double>>& as_matrix() const;

    // Numeric conversion
    double to_double() const;
    
    // String representation
    std::string to_string() const;
    
    // Arithmetic operations
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value power(const Value& other) const;
    
    // Matrix operations
    Value matrix_multiply(const Value& other) const;
    Value transpose() const;
    Value determinant() const;
    Value inverse() const;
    
    // Comparison operations
    Value operator==(const Value& other) const;
    Value operator!=(const Value& other) const;
    Value operator<(const Value& other) const;
    Value operator<=(const Value& other) const;
    Value operator>(const Value& other) const;
    Value operator>=(const Value& other) const;
    
    // Logical operations
    Value logical_and(const Value& other) const;
    Value logical_or(const Value& other) const;
    Value logical_not() const;
    
    // Unary operations
    Value negate() const;
    
    // Truth value for control flow
    bool is_truthy() const;
};

// Runtime exception for interpreter errors
class RuntimeError : public std::runtime_error {
private:
    size_t line_;
    size_t column_;

public:
    RuntimeError(const std::string& message, size_t line = 0, size_t column = 0)
        : std::runtime_error(message), line_(line), column_(column) {}
    
    size_t get_line() const { return line_; }
    size_t get_column() const { return column_; }
};

// Environment for variable storage
class Environment {
private:
    std::unordered_map<std::string, Value> variables_;
    std::shared_ptr<Environment> parent_;

public:
    Environment(std::shared_ptr<Environment> parent = nullptr) : parent_(parent) {}
    
    void define(const std::string& name, const Value& value);
    Value get(const std::string& name) const;
    void assign(const std::string& name, const Value& value);
    bool exists(const std::string& name) const;
    bool exists_in_current_scope(const std::string& name) const;
};

// Function representation
struct Function {
    std::string name;
    std::vector<std::string> parameters;
    uint32_t body_node_index;
    std::shared_ptr<Environment> closure;
    
    Function() : body_node_index(0) {}
    Function(const std::string& n, const std::vector<std::string>& params, 
             uint32_t body, std::shared_ptr<Environment> env)
        : name(n), parameters(params), body_node_index(body), closure(env) {}
};

// Built-in functions
class BuiltinFunctions {
public:
    static Value print(const std::vector<Value>& args);
    static Value input(const std::vector<Value>& args);
    static Value len(const std::vector<Value>& args);
    static Value abs(const std::vector<Value>& args);
    static Value sqrt(const std::vector<Value>& args);
    static Value sin(const std::vector<Value>& args);
    static Value cos(const std::vector<Value>& args);
    static Value tan(const std::vector<Value>& args);
    static Value pow(const std::vector<Value>& args);
    static Value floor(const std::vector<Value>& args);
    static Value ceil(const std::vector<Value>& args);
    static Value round(const std::vector<Value>& args);
    
    // Matrix functions
    static Value zeros(const std::vector<Value>& args);
    static Value ones(const std::vector<Value>& args);
    static Value eye(const std::vector<Value>& args);
    static Value transpose(const std::vector<Value>& args);
    static Value determinant(const std::vector<Value>& args);
    static Value inverse(const std::vector<Value>& args);
    
    // Range function for iteration
    static Value range(const std::vector<Value>& args);
};

// Return value exception for early returns
class ReturnException : public std::exception {
private:
    Value value_;

public:
    ReturnException(const Value& value) : value_(value) {}
    const Value& get_value() const { return value_; }
};

// Main interpreter class
class Interpreter {
private:
    const Parser& parser_;
    std::shared_ptr<Environment> global_env_;
    std::shared_ptr<Environment> current_env_;
    std::unordered_map<std::string, Function> user_functions_;
    std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>> builtin_functions_;
    
    // Helper methods
    Value evaluate_node(uint32_t node_index);
    Value evaluate_binary_op(const ASTNode& node);
    Value evaluate_unary_op(const ASTNode& node);
    Value evaluate_assignment(const ASTNode& node);
    Value evaluate_identifier(const ASTNode& node);
    Value evaluate_function_call(const ASTNode& node);
    Value evaluate_matrix_literal(const ASTNode& node);
    Value evaluate_matrix_access(const ASTNode& node);
    Value evaluate_member_access(const ASTNode& node);
    
    void execute_statement(uint32_t node_index);
    void execute_if_statement(const ASTNode& node);
    void execute_while_statement(const ASTNode& node);
    void execute_for_statement(const ASTNode& node);
    void execute_function_def(const ASTNode& node);
    void execute_return_statement(const ASTNode& node);
    void execute_block(uint32_t node_index);
    
    std::string get_node_string(uint32_t string_index) const;
    std::vector<uint32_t> get_child_indices(uint32_t node_index) const;
    
    void register_builtin_functions();
    
public:
    explicit Interpreter(const Parser& parser);
    
    // Main execution methods
    void interpret();
    Value interpret_expression(uint32_t node_index);
    
    // Environment access
    std::shared_ptr<Environment> get_global_environment() const { return global_env_; }
    std::shared_ptr<Environment> get_current_environment() const { return current_env_; }
    
    // Error handling
    void print_runtime_error(const RuntimeError& error) const;
};

} // namespace Dakota

#endif // INTERPRETER_H
