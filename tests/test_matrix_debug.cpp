#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>
#include <vector>

// Simple test to isolate matrix parsing issues
void test_minimal_matrix() {
    std::cout << "=== Minimal Matrix Test ===\n";
    
    try {
        std::string code = "[1, 2]";
        std::cout << "Testing: " << code << "\n";
        
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokens generated: " << tokens.size() << "\n";
        for (size_t i = 0; i < tokens.size(); i++) {
            std::cout << "  [" << i << "] " << static_cast<int>(tokens[i].type) 
                      << " '" << tokens[i].value << "'\n";
        }
        
        Dakota::Parser parser(tokens);
        std::cout << "Parser created\n";
        
        // Let's try to see what's happening step by step
        std::cout << "About to call parse()...\n";
        uint32_t root = parser.parse();
        std::cout << "Parse completed, root: " << root << "\n";
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "Parse successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    std::cout << "Test completed\n\n";
}

void test_empty_matrix() {
    std::cout << "=== Empty Matrix Test ===\n";
    
    try {
        std::string code = "[]";
        std::cout << "Testing: " << code << "\n";
        
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        std::cout << "Parser created\n";
        
        uint32_t root = parser.parse();
        std::cout << "Parse completed, root: " << root << "\n";
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "Parse successful!\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    std::cout << "Test completed\n\n";
}

int main() {
    std::cout << "Dakota Matrix Debug Tests\n";
    std::cout << "========================\n\n";
    
    test_empty_matrix();
    test_minimal_matrix();
    
    return 0;
}
