#include "src/lexer.h"
#include "src/parser.h"
#include <iostream>

int main() {
    std::cout << "Testing Dakota Parser Improvements\n";
    
    // Test 1: For loop parsing
    std::cout << "\nTest 1: For loop parsing\n";
    std::string for_code = "for i in range(10):\n    print(i)\n";
    
    Dakota::Lexer lexer(for_code);
    auto tokens = lexer.tokenize();
    
    Dakota::Parser parser(tokens);
    uint32_t root = parser.parse();
    
    if (parser.has_error()) {
        std::cout << "Parser error: " << parser.get_error() << "\n";
        return 1;
    }
    
    std::cout << "For loop AST:\n";
    parser.print_ast(root);
    
    // Test 2: Array access parsing
    std::cout << "\nTest 2: Array access parsing\n";
    std::string array_code = "arr[0] = 5\n";
    
    Dakota::Lexer lexer2(array_code);
    auto tokens2 = lexer2.tokenize();
    
    Dakota::Parser parser2(tokens2);
    uint32_t root2 = parser2.parse();
    
    if (parser2.has_error()) {
        std::cout << "Parser error: " << parser2.get_error() << "\n";
        return 1;
    }
    
    std::cout << "Array access AST:\n";
    parser2.print_ast(root2);
    
    // Test 3: Member access parsing
    std::cout << "\nTest 3: Member access parsing\n";
    std::string member_code = "obj.member = 10\n";
    
    Dakota::Lexer lexer3(member_code);
    auto tokens3 = lexer3.tokenize();
    
    Dakota::Parser parser3(tokens3);
    uint32_t root3 = parser3.parse();
    
    if (parser3.has_error()) {
        std::cout << "Parser error: " << parser3.get_error() << "\n";
        return 1;
    }
    
    std::cout << "Member access AST:\n";
    parser3.print_ast(root3);
    
    // Test 4: Error recovery
    std::cout << "\nTest 4: Error recovery\n";
    std::string error_code = "x = 1\n@invalid_token\ny = 2\n";
    
    Dakota::Lexer lexer4(error_code);
    auto tokens4 = lexer4.tokenize();
    
    Dakota::Parser parser4(tokens4);
    uint32_t root4 = parser4.parse();
    
    std::cout << "Error recovery AST (should have valid x and y assignments):\n";
    parser4.print_ast(root4);
    
    // Test 5: Memory usage
    std::cout << "\nTest 5: Memory usage\n";
    std::cout << "Memory usage: " << parser.get_memory_usage() << " bytes\n";
    
    std::cout << "\nAll tests completed successfully!\n";
    return 0;
}
