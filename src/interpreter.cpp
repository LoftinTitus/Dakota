#include "interpreter.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

namespace Dakota {

// Value class implementation

int64_t Value::as_integer() const {
    if (!is_integer()) {
        throw RuntimeError("Value is not an integer");
    }
    return std::get<int64_t>(value_);
}

double Value::as_float() const {
    if (!is_float()) {
        throw RuntimeError("Value is not a float");
    }
    return std::get<double>(value_);
}

const std::string& Value::as_string() const {
    if (!is_string()) {
        throw RuntimeError("Value is not a string");
    }
    return std::get<std::string>(value_);
}

bool Value::as_boolean() const {
    if (!is_boolean()) {
        throw RuntimeError("Value is not a boolean");
    }
    return std::get<bool>(value_);
}

const std::vector<std::vector<double>>& Value::as_matrix() const {
    if (!is_matrix()) {
        throw RuntimeError("Value is not a matrix");
    }
    return std::get<std::vector<std::vector<double>>>(value_);
}

double Value::to_double() const {
    if (is_integer()) {
        return static_cast<double>(as_integer());
    } else if (is_float()) {
        return as_float();
    } else {
        throw RuntimeError("Cannot convert value to double");
    }
}

std::string Value::to_string() const {
    switch (type_) {
        case Type::INTEGER:
            return std::to_string(as_integer());
        case Type::FLOAT: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << as_float();
            std::string result = oss.str();
            // Remove trailing zeros
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            if (result.back() == '.') result.pop_back();
            return result;
        }
        case Type::STRING:
            return as_string();
        case Type::BOOLEAN:
            return as_boolean() ? "true" : "false";
        case Type::MATRIX: {
            const auto& matrix = as_matrix();
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < matrix.size(); ++i) {
                if (i > 0) oss << ";";
                for (size_t j = 0; j < matrix[i].size(); ++j) {
                    if (j > 0) oss << ",";
                    oss << matrix[i][j];
                }
            }
            oss << "]";
            return oss.str();
        }
        case Type::NONE:
            return "none";
        default:
            return "unknown";
    }
}

Value Value::operator+(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        if (is_integer() && other.is_integer()) {
            return Value(as_integer() + other.as_integer());
        } else {
            return Value(to_double() + other.to_double());
        }
    } else if (is_string() && other.is_string()) {
        return Value(as_string() + other.as_string());
    } else if (is_matrix() && other.is_matrix()) {
        const auto& a = as_matrix();
        const auto& b = other.as_matrix();
        if (a.size() != b.size() || (a.size() > 0 && a[0].size() != b[0].size())) {
            throw RuntimeError("Matrix dimensions don't match for addition");
        }
        
        std::vector<std::vector<double>> result(a.size());
        for (size_t i = 0; i < a.size(); ++i) {
            result[i].resize(a[i].size());
            for (size_t j = 0; j < a[i].size(); ++j) {
                result[i][j] = a[i][j] + b[i][j];
            }
        }
        return Value(result);
    }
    throw RuntimeError("Cannot add values of these types");
}

Value Value::operator-(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        if (is_integer() && other.is_integer()) {
            return Value(as_integer() - other.as_integer());
        } else {
            return Value(to_double() - other.to_double());
        }
    } else if (is_matrix() && other.is_matrix()) {
        const auto& a = as_matrix();
        const auto& b = other.as_matrix();
        if (a.size() != b.size() || (a.size() > 0 && a[0].size() != b[0].size())) {
            throw RuntimeError("Matrix dimensions don't match for subtraction");
        }
        
        std::vector<std::vector<double>> result(a.size());
        for (size_t i = 0; i < a.size(); ++i) {
            result[i].resize(a[i].size());
            for (size_t j = 0; j < a[i].size(); ++j) {
                result[i][j] = a[i][j] - b[i][j];
            }
        }
        return Value(result);
    }
    throw RuntimeError("Cannot subtract values of these types");
}

Value Value::operator*(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        if (is_integer() && other.is_integer()) {
            return Value(as_integer() * other.as_integer());
        } else {
            return Value(to_double() * other.to_double());
        }
    } else if (is_matrix() && other.is_numeric()) {
        const auto& matrix = as_matrix();
        double scalar = other.to_double();
        
        std::vector<std::vector<double>> result(matrix.size());
        for (size_t i = 0; i < matrix.size(); ++i) {
            result[i].resize(matrix[i].size());
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                result[i][j] = matrix[i][j] * scalar;
            }
        }
        return Value(result);
    } else if (is_numeric() && other.is_matrix()) {
        return other * (*this); // Commutative
    }
    throw RuntimeError("Cannot multiply values of these types");
}

Value Value::operator/(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        double divisor = other.to_double();
        if (divisor == 0.0) {
            throw RuntimeError("Division by zero");
        }
        return Value(to_double() / divisor);
    } else if (is_matrix() && other.is_numeric()) {
        const auto& matrix = as_matrix();
        double scalar = other.to_double();
        if (scalar == 0.0) {
            throw RuntimeError("Division by zero");
        }
        
        std::vector<std::vector<double>> result(matrix.size());
        for (size_t i = 0; i < matrix.size(); ++i) {
            result[i].resize(matrix[i].size());
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                result[i][j] = matrix[i][j] / scalar;
            }
        }
        return Value(result);
    }
    throw RuntimeError("Cannot divide values of these types");
}

Value Value::operator%(const Value& other) const {
    if (is_integer() && other.is_integer()) {
        int64_t divisor = other.as_integer();
        if (divisor == 0) {
            throw RuntimeError("Modulo by zero");
        }
        return Value(as_integer() % divisor);
    }
    throw RuntimeError("Modulo operation requires integer operands");
}

Value Value::power(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        return Value(std::pow(to_double(), other.to_double()));
    }
    throw RuntimeError("Power operation requires numeric operands");
}

Value Value::matrix_multiply(const Value& other) const {
    if (!is_matrix() || !other.is_matrix()) {
        throw RuntimeError("Matrix multiplication requires matrix operands");
    }
    
    const auto& a = as_matrix();
    const auto& b = other.as_matrix();
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        throw RuntimeError("Invalid matrix dimensions for multiplication");
    }
    
    size_t rows = a.size();
    size_t cols = b[0].size();
    size_t inner = a[0].size();
    
    std::vector<std::vector<double>> result(rows, std::vector<double>(cols, 0.0));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            for (size_t k = 0; k < inner; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    
    return Value(result);
}

Value Value::transpose() const {
    if (!is_matrix()) {
        throw RuntimeError("Transpose operation requires a matrix");
    }
    
    const auto& matrix = as_matrix();
    if (matrix.empty()) {
        return Value(std::vector<std::vector<double>>());
    }
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    std::vector<std::vector<double>> result(cols, std::vector<double>(rows));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return Value(result);
}

Value Value::determinant() const {
    if (!is_matrix()) {
        throw RuntimeError("Determinant operation requires a matrix");
    }
    
    const auto& matrix = as_matrix();
    if (matrix.empty() || matrix.size() != matrix[0].size()) {
        throw RuntimeError("Determinant requires a square matrix");
    }
    
    size_t n = matrix.size();
    if (n == 1) {
        return Value(matrix[0][0]);
    } else if (n == 2) {
        return Value(matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]);
    }
    
    // For larger matrices, use LU decomposition or recursive expansion
    // This is a simplified implementation using recursive expansion
    double det = 0.0;
    for (size_t j = 0; j < n; ++j) {
        // Create minor matrix
        std::vector<std::vector<double>> minor(n - 1, std::vector<double>(n - 1));
        for (size_t i = 1; i < n; ++i) {
            size_t col_idx = 0;
            for (size_t k = 0; k < n; ++k) {
                if (k != j) {
                    minor[i - 1][col_idx++] = matrix[i][k];
                }
            }
        }
        
        Value minor_det = Value(minor).determinant();
        det += (j % 2 == 0 ? 1 : -1) * matrix[0][j] * minor_det.to_double();
    }
    
    return Value(det);
}

Value Value::inverse() const {
    if (!is_matrix()) {
        throw RuntimeError("Inverse operation requires a matrix");
    }
    
    const auto& matrix = as_matrix();
    if (matrix.empty() || matrix.size() != matrix[0].size()) {
        throw RuntimeError("Inverse requires a square matrix");
    }
    
    size_t n = matrix.size();
    
    // Create augmented matrix [A|I] where I is the identity matrix
    std::vector<std::vector<double>> augmented(n, std::vector<double>(2 * n));
    
    // Fill the augmented matrix
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            augmented[i][j] = matrix[i][j];           // Original matrix
            augmented[i][j + n] = (i == j) ? 1.0 : 0.0; // Identity matrix
        }
    }
    
    // Gaussian elimination with partial pivoting
    for (size_t i = 0; i < n; ++i) {
        // Find pivot
        size_t pivot_row = i;
        for (size_t k = i + 1; k < n; ++k) {
            if (std::abs(augmented[k][i]) > std::abs(augmented[pivot_row][i])) {
                pivot_row = k;
            }
        }
        
        // Check for singularity
        if (std::abs(augmented[pivot_row][i]) < 1e-10) {
            throw RuntimeError("Matrix is singular (not invertible)");
        }
        
        // Swap rows if needed
        if (pivot_row != i) {
            std::swap(augmented[i], augmented[pivot_row]);
        }
        
        // Scale pivot row
        double pivot = augmented[i][i];
        for (size_t j = 0; j < 2 * n; ++j) {
            augmented[i][j] /= pivot;
        }
        
        // Eliminate column
        for (size_t k = 0; k < n; ++k) {
            if (k != i) {
                double factor = augmented[k][i];
                for (size_t j = 0; j < 2 * n; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }
    
    // Extract the inverse matrix from the right half
    std::vector<std::vector<double>> result(n, std::vector<double>(n));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            result[i][j] = augmented[i][j + n];
        }
    }
    
    return Value(result);
}

Value Value::operator==(const Value& other) const {
    if (type_ != other.type_) return Value(false);
    
    switch (type_) {
        case Type::INTEGER: return Value(as_integer() == other.as_integer());
        case Type::FLOAT: return Value(std::abs(as_float() - other.as_float()) < 1e-10);
        case Type::STRING: return Value(as_string() == other.as_string());
        case Type::BOOLEAN: return Value(as_boolean() == other.as_boolean());
        case Type::MATRIX: return Value(as_matrix() == other.as_matrix());
        case Type::NONE: return Value(true);
        default: return Value(false);
    }
}

Value Value::operator!=(const Value& other) const {
    return Value(!(*this == other).as_boolean());
}

Value Value::operator<(const Value& other) const {
    if (is_numeric() && other.is_numeric()) {
        return Value(to_double() < other.to_double());
    } else if (is_string() && other.is_string()) {
        return Value(as_string() < other.as_string());
    }
    throw RuntimeError("Cannot compare values of these types");
}

Value Value::operator<=(const Value& other) const {
    return Value((*this < other).as_boolean() || (*this == other).as_boolean());
}

Value Value::operator>(const Value& other) const {
    return Value(!(*this <= other).as_boolean());
}

Value Value::operator>=(const Value& other) const {
    return Value(!(*this < other).as_boolean());
}

Value Value::logical_and(const Value& other) const {
    return Value(is_truthy() && other.is_truthy());
}

Value Value::logical_or(const Value& other) const {
    return Value(is_truthy() || other.is_truthy());
}

Value Value::logical_not() const {
    return Value(!is_truthy());
}

Value Value::negate() const {
    if (is_integer()) {
        return Value(-as_integer());
    } else if (is_float()) {
        return Value(-as_float());
    } else if (is_matrix()) {
        const auto& matrix = as_matrix();
        std::vector<std::vector<double>> result(matrix.size());
        for (size_t i = 0; i < matrix.size(); ++i) {
            result[i].resize(matrix[i].size());
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                result[i][j] = -matrix[i][j];
            }
        }
        return Value(result);
    }
    throw RuntimeError("Cannot negate this value type");
}

bool Value::is_truthy() const {
    switch (type_) {
        case Type::INTEGER: return as_integer() != 0;
        case Type::FLOAT: return as_float() != 0.0;
        case Type::STRING: return !as_string().empty();
        case Type::BOOLEAN: return as_boolean();
        case Type::MATRIX: return !as_matrix().empty();
        case Type::NONE: return false;
        default: return false;
    }
}

// Environment implementation

void Environment::define(const std::string& name, const Value& value) {
    variables_[name] = value;
}

Value Environment::get(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    
    if (parent_) {
        return parent_->get(name);
    }
    
    throw RuntimeError("Undefined variable '" + name + "'");
}

void Environment::assign(const std::string& name, const Value& value) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        it->second = value;
        return;
    }
    
    if (parent_) {
        try {
            parent_->assign(name, value);
            return;
        } catch (const RuntimeError&) {
            // Fall through to define in current scope
        }
    }
    
    // Variable doesn't exist, create it in current scope
    variables_[name] = value;
}

bool Environment::exists(const std::string& name) const {
    if (variables_.find(name) != variables_.end()) {
        return true;
    }
    return parent_ && parent_->exists(name);
}

bool Environment::exists_in_current_scope(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

// Built-in functions implementation

Value BuiltinFunctions::print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].to_string();
    }
    std::cout << std::endl;
    return Value(); // None
}

Value BuiltinFunctions::input(const std::vector<Value>& args) {
    if (!args.empty()) {
        std::cout << args[0].to_string();
    }
    
    std::string line;
    std::getline(std::cin, line);
    return Value(line);
}

Value BuiltinFunctions::len(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("len() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (val.is_string()) {
        return Value(static_cast<int64_t>(val.as_string().length()));
    } else if (val.is_matrix()) {
        return Value(static_cast<int64_t>(val.as_matrix().size()));
    }
    
    throw RuntimeError("len() argument must be a string or matrix");
}

Value BuiltinFunctions::abs(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("abs() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (val.is_integer()) {
        return Value(std::abs(val.as_integer()));
    } else if (val.is_float()) {
        return Value(std::abs(val.as_float()));
    }
    
    throw RuntimeError("abs() argument must be numeric");
}

Value BuiltinFunctions::sqrt(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("sqrt() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("sqrt() argument must be numeric");
    }
    
    return Value(std::sqrt(val.to_double()));
}

Value BuiltinFunctions::sin(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("sin() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("sin() argument must be numeric");
    }
    
    return Value(std::sin(val.to_double()));
}

Value BuiltinFunctions::cos(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("cos() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("cos() argument must be numeric");
    }
    
    return Value(std::cos(val.to_double()));
}

Value BuiltinFunctions::tan(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("tan() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("tan() argument must be numeric");
    }
    
    return Value(std::tan(val.to_double()));
}

Value BuiltinFunctions::pow(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw RuntimeError("pow() takes exactly two arguments");
    }
    
    const Value& base = args[0];
    const Value& exponent = args[1];
    
    if (!base.is_numeric() || !exponent.is_numeric()) {
        throw RuntimeError("pow() arguments must be numeric");
    }
    
    return Value(std::pow(base.to_double(), exponent.to_double()));
}

Value BuiltinFunctions::floor(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("floor() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("floor() argument must be numeric");
    }
    
    return Value(std::floor(val.to_double()));
}

Value BuiltinFunctions::ceil(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("ceil() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("ceil() argument must be numeric");
    }
    
    return Value(std::ceil(val.to_double()));
}

Value BuiltinFunctions::round(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("round() takes exactly one argument");
    }
    
    const Value& val = args[0];
    if (!val.is_numeric()) {
        throw RuntimeError("round() argument must be numeric");
    }
    
    return Value(std::round(val.to_double()));
}

Value BuiltinFunctions::zeros(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw RuntimeError("zeros() takes exactly two arguments (rows, cols)");
    }
    
    if (!args[0].is_integer() || !args[1].is_integer()) {
        throw RuntimeError("zeros() arguments must be integers");
    }
    
    int64_t rows = args[0].as_integer();
    int64_t cols = args[1].as_integer();
    
    if (rows < 0 || cols < 0) {
        throw RuntimeError("Matrix dimensions must be non-negative");
    }
    
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols, 0.0));
    return Value(matrix);
}

Value BuiltinFunctions::ones(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw RuntimeError("ones() takes exactly two arguments (rows, cols)");
    }
    
    if (!args[0].is_integer() || !args[1].is_integer()) {
        throw RuntimeError("ones() arguments must be integers");
    }
    
    int64_t rows = args[0].as_integer();
    int64_t cols = args[1].as_integer();
    
    if (rows < 0 || cols < 0) {
        throw RuntimeError("Matrix dimensions must be non-negative");
    }
    
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols, 1.0));
    return Value(matrix);
}

Value BuiltinFunctions::eye(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("eye() takes exactly one argument (size)");
    }
    
    if (!args[0].is_integer()) {
        throw RuntimeError("eye() argument must be an integer");
    }
    
    int64_t size = args[0].as_integer();
    
    if (size < 0) {
        throw RuntimeError("Matrix size must be non-negative");
    }
    
    std::vector<std::vector<double>> matrix(size, std::vector<double>(size, 0.0));
    for (int64_t i = 0; i < size; ++i) {
        matrix[i][i] = 1.0;
    }
    
    return Value(matrix);
}

Value BuiltinFunctions::transpose(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("transpose() takes exactly one argument");
    }
    
    return args[0].transpose();
}

Value BuiltinFunctions::determinant(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("determinant() takes exactly one argument");
    }
    
    return args[0].determinant();
}

Value BuiltinFunctions::inverse(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw RuntimeError("inverse() takes exactly one argument");
    }
    
    return args[0].inverse();
}

// Interpreter implementation

Interpreter::Interpreter(const Parser& parser) 
    : parser_(parser), global_env_(std::make_shared<Environment>()), current_env_(global_env_) {
    register_builtin_functions();
}

void Interpreter::register_builtin_functions() {
    builtin_functions_["print"] = BuiltinFunctions::print;
    builtin_functions_["input"] = BuiltinFunctions::input;
    builtin_functions_["len"] = BuiltinFunctions::len;
    builtin_functions_["abs"] = BuiltinFunctions::abs;
    builtin_functions_["sqrt"] = BuiltinFunctions::sqrt;
    builtin_functions_["sin"] = BuiltinFunctions::sin;
    builtin_functions_["cos"] = BuiltinFunctions::cos;
    builtin_functions_["tan"] = BuiltinFunctions::tan;
    builtin_functions_["pow"] = BuiltinFunctions::pow;
    builtin_functions_["floor"] = BuiltinFunctions::floor;
    builtin_functions_["ceil"] = BuiltinFunctions::ceil;
    builtin_functions_["round"] = BuiltinFunctions::round;
    builtin_functions_["zeros"] = BuiltinFunctions::zeros;
    builtin_functions_["ones"] = BuiltinFunctions::ones;
    builtin_functions_["eye"] = BuiltinFunctions::eye;
    builtin_functions_["transpose"] = BuiltinFunctions::transpose;
    builtin_functions_["determinant"] = BuiltinFunctions::determinant;
    builtin_functions_["inverse"] = BuiltinFunctions::inverse;
}

void Interpreter::interpret() {
    if (parser_.has_error()) {
        throw RuntimeError("Cannot interpret: parser has errors");
    }
    
    const auto& nodes = parser_.get_nodes();
    if (nodes.empty()) {
        return;
    }
    
    std::cout << "DEBUG: Starting interpretation with " << nodes.size() << " nodes\n";
    
    try {
        // Execute the program starting from the root node
        std::cout << "DEBUG: Executing statement at index 0\n";
        execute_statement(0);
        std::cout << "DEBUG: Interpretation completed\n";
    } catch (const ReturnException& ret) {
        // Return from main program - this is fine
        std::cout << "DEBUG: Return exception caught\n";
    } catch (const RuntimeError& e) {
        print_runtime_error(e);
        throw;
    }
}

Value Interpreter::interpret_expression(uint32_t node_index) {
    return evaluate_node(node_index);
}

Value Interpreter::evaluate_node(uint32_t node_index) {
    if (node_index == INVALID_INDEX || node_index >= parser_.get_nodes().size()) {
        throw RuntimeError("Invalid node index");
    }
    
    const ASTNode& node = parser_.get_nodes()[node_index];
    
    switch (node.type) {
        case NodeType::INTEGER_LITERAL:
            return Value(node.integer_literal.value);
            
        case NodeType::FLOAT_LITERAL:
            return Value(node.float_literal.value);
            
        case NodeType::STRING_LITERAL:
            return Value(get_node_string(node.string_literal.string_index));
            
        case NodeType::BOOLEAN_LITERAL:
            return Value(node.boolean_literal.value);
            
        case NodeType::IDENTIFIER:
            return evaluate_identifier(node);
            
        case NodeType::BINARY_OP:
            return evaluate_binary_op(node);
            
        case NodeType::UNARY_OP:
            return evaluate_unary_op(node);
            
        case NodeType::ASSIGNMENT:
            return evaluate_assignment(node);
            
        case NodeType::FUNCTION_CALL:
            return evaluate_function_call(node);
            
        case NodeType::MATRIX_LITERAL:
            return evaluate_matrix_literal(node);
            
        case NodeType::MATRIX_ACCESS:
            return evaluate_matrix_access(node);
            
        case NodeType::MEMBER_ACCESS:
            return evaluate_member_access(node);
            
        default:
            throw RuntimeError("Cannot evaluate node type as expression");
    }
}

Value Interpreter::evaluate_binary_op(const ASTNode& node) {
    Value left = evaluate_node(node.binary_op.left_index);
    Value right = evaluate_node(node.binary_op.right_index);
    
    switch (node.binary_op.op_type) {
        case BinaryOpType::ADD:
            return left + right;
        case BinaryOpType::SUB:
            return left - right;
        case BinaryOpType::MUL:
            return left * right;
        case BinaryOpType::DIV:
            return left / right;
        case BinaryOpType::POW:
            return left.power(right);
        case BinaryOpType::MATMUL:
            return left.matrix_multiply(right);
        case BinaryOpType::EQ:
            return left == right;
        case BinaryOpType::NE:
            return left != right;
        case BinaryOpType::LT:
            return left < right;
        case BinaryOpType::LE:
            return left <= right;
        case BinaryOpType::GT:
            return left > right;
        case BinaryOpType::GE:
            return left >= right;
        case BinaryOpType::AND:
            return left.logical_and(right);
        case BinaryOpType::OR:
            return left.logical_or(right);
        default:
            throw RuntimeError("Unknown binary operator");
    }
}

Value Interpreter::evaluate_unary_op(const ASTNode& node) {
    Value operand = evaluate_node(node.unary_op.operand_index);
    
    switch (node.unary_op.op_type) {
        case UnaryOpType::NEGATE:
            return operand.negate();
        case UnaryOpType::NOT:
            return operand.logical_not();
        default:
            throw RuntimeError("Unknown unary operator");
    }
}

Value Interpreter::evaluate_assignment(const ASTNode& node) {
    Value value = evaluate_node(node.assignment.value_index);
    
    const ASTNode& target = parser_.get_nodes()[node.assignment.target_index];
    if (target.type != NodeType::IDENTIFIER) {
        throw RuntimeError("Invalid assignment target");
    }
    
    std::string name = get_node_string(target.identifier.name_index);
    current_env_->assign(name, value);
    
    return value;
}

Value Interpreter::evaluate_identifier(const ASTNode& node) {
    std::string name = get_node_string(node.identifier.name_index);
    return current_env_->get(name);
}

Value Interpreter::evaluate_function_call(const ASTNode& node) {
    std::string function_name = get_node_string(node.function_call.name_index);
    
    // Collect arguments
    std::vector<Value> args;
    std::vector<uint32_t> arg_indices = get_child_indices(node.function_call.args_start_index);
    
    for (uint32_t arg_index : arg_indices) {
        args.push_back(evaluate_node(arg_index));
    }
    
    // Check for built-in functions first
    auto builtin_it = builtin_functions_.find(function_name);
    if (builtin_it != builtin_functions_.end()) {
        return builtin_it->second(args);
    }
    
    // Check for user-defined functions
    auto user_it = user_functions_.find(function_name);
    if (user_it != user_functions_.end()) {
        const Function& func = user_it->second;
        
        if (args.size() != func.parameters.size()) {
            throw RuntimeError("Function '" + function_name + "' expects " + 
                             std::to_string(func.parameters.size()) + " arguments, got " +
                             std::to_string(args.size()));
        }
        
        // Create new environment for function execution
        auto func_env = std::make_shared<Environment>(func.closure);
        
        // Bind parameters to arguments
        // Note: Parameters shadow any global variables with the same name
        for (size_t i = 0; i < func.parameters.size(); ++i) {
            func_env->define(func.parameters[i], args[i]);
        }
        
        // Execute function body
        auto previous_env = current_env_;
        current_env_ = func_env;
        
        try {
            execute_statement(func.body_node_index);
            current_env_ = previous_env;
            return Value(); // No explicit return
        } catch (const ReturnException& ret) {
            current_env_ = previous_env;
            return ret.get_value();
        }
    }
    
    throw RuntimeError("Undefined function '" + function_name + "'");
}

Value Interpreter::evaluate_matrix_literal(const ASTNode& node) {
    std::vector<std::vector<double>> matrix;
    
    if (node.matrix_literal.is_empty) {
        return Value(matrix);
    }
    
    // Get all element nodes
    std::vector<uint32_t> element_indices = get_child_indices(node.matrix_literal.elements_start_index);
    
    size_t rows = node.matrix_literal.rows;
    size_t cols = node.matrix_literal.cols;
    
    matrix.resize(rows);
    for (size_t i = 0; i < rows; ++i) {
        matrix[i].resize(cols);
    }
    
    // Fill matrix with values
    for (size_t i = 0; i < element_indices.size() && i < rows * cols; ++i) {
        Value element_value = evaluate_node(element_indices[i]);
        if (!element_value.is_numeric()) {
            throw RuntimeError("Matrix elements must be numeric");
        }
        
        size_t row = i / cols;
        size_t col = i % cols;
        matrix[row][col] = element_value.to_double();
    }
    
    return Value(matrix);
}

Value Interpreter::evaluate_matrix_access(const ASTNode& node) {
    Value matrix_val = evaluate_node(node.matrix_access.object_index);
    Value index_val = evaluate_node(node.matrix_access.index_index);
    
    if (!matrix_val.is_matrix()) {
        throw RuntimeError("Cannot index non-matrix value");
    }
    
    if (!index_val.is_integer()) {
        throw RuntimeError("Matrix index must be an integer");
    }
    
    const auto& matrix = matrix_val.as_matrix();
    int64_t index = index_val.as_integer();
    
    if (index < 0 || static_cast<size_t>(index) >= matrix.size()) {
        throw RuntimeError("Matrix index out of bounds");
    }
    
    // Return the row as a new matrix (vector)
    std::vector<std::vector<double>> row_matrix;
    row_matrix.push_back(matrix[index]);
    return Value(row_matrix);
}

Value Interpreter::evaluate_member_access(const ASTNode& node) {
    Value object = evaluate_node(node.member_access.object_index);
    std::string member_name = get_node_string(node.member_access.member_name_index);
    
    if (object.is_matrix()) {
        if (member_name == "T") {
            return object.transpose();
        } else if (member_name == "d") {
            return object.determinant();
        } else if (member_name == "I") {
            return object.inverse();
        }
    }
    
    throw RuntimeError("Unknown member '" + member_name + "' for this type");
}

void Interpreter::execute_statement(uint32_t node_index) {
    if (node_index == INVALID_INDEX || node_index >= parser_.get_nodes().size()) {
        return;
    }
    
    const ASTNode& node = parser_.get_nodes()[node_index];
    
    switch (node.type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK:
            execute_block(node_index);
            break;
            
        case NodeType::EXPRESSION_STATEMENT:
            // Execute the child expression and discard result
            if (node.first_child_index != INVALID_INDEX) {
                evaluate_node(node.first_child_index);
            }
            break;
            
        case NodeType::IF_STATEMENT:
            execute_if_statement(node);
            break;
            
        case NodeType::WHILE_STATEMENT:
            execute_while_statement(node);
            break;
            
        case NodeType::FOR_STATEMENT:
            execute_for_statement(node);
            break;
            
        case NodeType::FUNCTION_DEF:
            execute_function_def(node);
            break;
            
        case NodeType::RETURN_STATEMENT:
            execute_return_statement(node);
            break;
            
        case NodeType::ASSIGNMENT:
            evaluate_assignment(node);
            break;
            
        default:
            throw RuntimeError("Unknown or unsupported statement node type: " + std::to_string(static_cast<int>(node.type)));
    }
}

void Interpreter::execute_if_statement(const ASTNode& node) {
    Value condition = evaluate_node(node.if_statement.condition_index);
    
    if (condition.is_truthy()) {
        execute_statement(node.if_statement.then_block_index);
    } else if (node.if_statement.else_block_index != INVALID_INDEX) {
        execute_statement(node.if_statement.else_block_index);
    }
}

void Interpreter::execute_while_statement(const ASTNode& node) {
    while (true) {
        Value condition = evaluate_node(node.while_statement.condition_index);
        if (!condition.is_truthy()) {
            break;
        }
        
        execute_statement(node.while_statement.body_index);
    }
}

void Interpreter::execute_for_statement(const ASTNode& /* node */) {
    // Simplified for loop implementation
    // In a full implementation, you'd handle iterables properly
    throw RuntimeError("For loops not yet fully implemented");
}

void Interpreter::execute_function_def(const ASTNode& node) {
    std::string function_name = get_node_string(node.function_def.name_index);
    
    // Collect parameter names
    std::vector<std::string> parameters;
    std::vector<uint32_t> param_indices = get_child_indices(node.function_def.params_start_index);
    
    for (uint32_t param_index : param_indices) {
        const ASTNode& param_node = parser_.get_nodes()[param_index];
        if (param_node.type != NodeType::IDENTIFIER) {
            throw RuntimeError("Function parameter must be an identifier");
        }
        parameters.push_back(get_node_string(param_node.identifier.name_index));
    }
    
    // Create function object
    Function func(function_name, parameters, node.function_def.body_index, current_env_);
    user_functions_[function_name] = func;
}

void Interpreter::execute_return_statement(const ASTNode& node) {
    Value return_value;
    if (node.return_statement.value_index != INVALID_INDEX) {
        return_value = evaluate_node(node.return_statement.value_index);
    }
    
    throw ReturnException(return_value);
}

void Interpreter::execute_block(uint32_t node_index) {
    const ASTNode& node = parser_.get_nodes()[node_index];
    
    // Get all child statements
    std::vector<uint32_t> statement_indices = get_child_indices(node.first_child_index);
    
    for (uint32_t stmt_index : statement_indices) {
        execute_statement(stmt_index);
    }
}

std::string Interpreter::get_node_string(uint32_t string_index) const {
    return std::string(parser_.get_strings().get_string(string_index));
}

std::vector<uint32_t> Interpreter::get_child_indices(uint32_t start_index) const {
    std::vector<uint32_t> indices;
    
    if (start_index == INVALID_INDEX) {
        return indices;
    }
    
    const auto& nodes = parser_.get_nodes();
    uint32_t current = start_index;
    
    while (current != INVALID_INDEX && current < nodes.size()) {
        indices.push_back(current);
        current = nodes[current].next_sibling_index;
    }
    
    return indices;
}

void Interpreter::print_runtime_error(const RuntimeError& error) const {
    std::cerr << "Runtime Error";
    if (error.get_line() > 0) {
        std::cerr << " at line " << error.get_line();
        if (error.get_column() > 0) {
            std::cerr << ", column " << error.get_column();
        }
    }
    std::cerr << ": " << error.what() << std::endl;
}

} // namespace Dakota
