#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// Helper function to tokenize and parse matrix expressions
Dakota::Parser parse_matrix_code(const std::string& code) {
    Dakota::Lexer lexer(code, 4, false);
    auto tokens = lexer.tokenize();
    return Dakota::Parser(tokens);
}

// Helper function to print detailed token information
void print_tokens(const std::vector<Dakota::Token>& tokens) {
    std::cout << "Tokens (" << tokens.size() << "):\n";
    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout << "  [" << i << "] Type: " << static_cast<int>(tokens[i].type) 
                  << " Value: '" << tokens[i].value << "' Line: " << tokens[i].line << "\n";
    }
    std::cout << "\n";
}

// Helper function to print AST structure for debugging
void print_matrix_ast(const Dakota::Parser& parser, uint32_t node_index = 0, int depth = 0) {
    const auto& nodes = parser.get_nodes();
    if (node_index >= nodes.size()) return;
    
    const auto& node = nodes[node_index];
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    
    // Print node type and details
    std::cout << "Node[" << node_index << "] Type: " << static_cast<int>(node.type);
    
    if (node.type == Dakota::NodeType::MATRIX_LITERAL) {
        std::cout << " (MATRIX_LITERAL) rows=" << node.matrix_literal.rows 
                  << " cols=" << node.matrix_literal.cols
                  << " elements_start=" << node.matrix_literal.elements_start_index
                  << " empty=" << node.matrix_literal.is_empty;
    } else if (node.type == Dakota::NodeType::INTEGER_LITERAL) {
        std::cout << " (INTEGER_LITERAL) value=" << node.integer_literal.value;
    } else if (node.type == Dakota::NodeType::FLOAT_LITERAL) {
        std::cout << " (FLOAT_LITERAL) value=" << node.float_literal.value;
    }
    
    std::cout << " parent=" << node.parent_index 
              << " first_child=" << node.first_child_index 
              << " next_sibling=" << node.next_sibling_index << "\n";
    
    // Recursively print children
    if (node.first_child_index != Dakota::INVALID_INDEX) {
        print_matrix_ast(parser, node.first_child_index, depth + 1);
    }
    
    // Print siblings
    if (node.next_sibling_index != Dakota::INVALID_INDEX) {
        print_matrix_ast(parser, node.next_sibling_index, depth);
    }
}

void test_simple_matrix() {
    std::cout << "\n=== Test Simple Matrix ===\n";
    std::string code = "matrix = [1, 2; 3, 4]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        // Print tokens for debugging
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        print_tokens(tokens);
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_single_row_matrix() {
    std::cout << "\n=== Test Single Row Matrix ===\n";
    std::string code = "row_matrix = [1, 2, 3, 4]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_single_column_matrix() {
    std::cout << "\n=== Test Single Column Matrix ===\n";
    std::string code = "col_matrix = [1; 2; 3; 4]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_empty_matrix() {
    std::cout << "\n=== Test Empty Matrix ===\n";
    std::string code = "empty_matrix = []";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_matrix_with_floats() {
    std::cout << "\n=== Test Matrix with Floats ===\n";
    std::string code = "float_matrix = [1.5, 2.7; 3.14, 4.0]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_matrix_with_expressions() {
    std::cout << "\n=== Test Matrix with Expressions ===\n";
    std::string code = "expr_matrix = [1+2, 3*4; 5-1, 6/2]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_large_matrix() {
    std::cout << "\n=== Test Large Matrix ===\n";
    std::string code = "large_matrix = [1, 2, 3; 4, 5, 6; 7, 8, 9; 10, 11, 12]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_inconsistent_matrix_rows() {
    std::cout << "\n=== Test Inconsistent Matrix Rows (Should Fail) ===\n";
    std::string code = "bad_matrix = [1, 2; 3, 4, 5]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "✅ Expected parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "❌ Should have failed with inconsistent row lengths!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "✅ Expected exception: " << e.what() << "\n";
    }
}

void test_missing_closing_bracket() {
    std::cout << "\n=== Test Missing Closing Bracket (Should Fail) ===\n";
    std::string code = "bad_matrix = [1, 2; 3, 4";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "✅ Expected parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "❌ Should have failed with missing closing bracket!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "✅ Expected exception: " << e.what() << "\n";
    }
}

void test_nested_matrix_expressions() {
    std::cout << "\n=== Test Matrix in Variable Assignment ===\n";
    std::string code = R"(
x = 5
y = 10
matrix = [x, y; x+y, x*y]
result = matrix mult [1; 2]
)";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_matrix_multiplication() {
    std::cout << "\n=== Test Matrix Multiplication ===\n";
    std::string code = "result = [1, 2; 3, 4] mult [5, 6; 7, 8]";
    
    try {
        auto parser = parse_matrix_code(code);
        
        std::cout << "Code: " << code << "\n";
        
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            print_matrix_ast(parser);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Dakota Matrix Parsing Test Suite\n";
    std::cout << "================================\n";
    
    // Basic matrix tests
    test_simple_matrix();
    test_single_row_matrix();
    test_single_column_matrix();
    test_empty_matrix();
    
    // Matrix with different data types
    test_matrix_with_floats();
    test_matrix_with_expressions();
    test_large_matrix();
    
    // Error cases
    test_inconsistent_matrix_rows();
    test_missing_closing_bracket();
    
    // Complex scenarios
    test_nested_matrix_expressions();
    test_matrix_multiplication();
    
    std::cout << "\n=== Test Suite Complete ===\n";
    return 0;
}
