#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>

void test_matrix_literal_only() {
    std::cout << "=== Matrix Literal Only Test ===\n";
    
    try {
        std::string code = "[1, 2; 3, 4]";
        std::cout << "Testing: " << code << "\n";
        
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_simple_assignment() {
    std::cout << "\n=== Simple Assignment Test ===\n";
    
    try {
        std::string code = "x = 42";
        std::cout << "Testing: " << code << "\n";
        
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

void test_matrix_assignment() {
    std::cout << "\n=== Matrix Assignment Test ===\n";
    
    try {
        std::string code = "matrix = [1, 2]";
        std::cout << "Testing: " << code << "\n";
        
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        uint32_t root = parser.parse();
        
        if (parser.has_error()) {
            std::cout << "❌ Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "✅ Parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Dakota Matrix Isolation Tests\n";
    std::cout << "============================\n\n";
    
    test_matrix_literal_only();
    test_simple_assignment();
    test_matrix_assignment();
    
    return 0;
}
