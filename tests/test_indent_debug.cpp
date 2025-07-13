#include "src/lexer.h"
#include <iostream>

int main() {
    std::string test_code = R"(function test():
    x = 1
    return x

y = 2)";

    std::cout << "Test code:\n" << test_code << "\n\n";
    
    try {
        Dakota::Lexer lexer(test_code, 4, false);
        auto tokens = lexer.tokenize();
        
        std::cout << "Generated " << tokens.size() << " tokens:\n";
        
        for (size_t i = 0; i < tokens.size(); i++) {
            std::cout << i << ": ";
            
            switch (tokens[i].type) {
                case Dakota::TokenType::NEWLINE: std::cout << "NEWLINE"; break;
                case Dakota::TokenType::INDENT: std::cout << "INDENT"; break;
                case Dakota::TokenType::DEDENT: std::cout << "DEDENT"; break;
                case Dakota::TokenType::FUNCTION: std::cout << "FUNCTION"; break;
                case Dakota::TokenType::IDENTIFIER: std::cout << "IDENTIFIER"; break;
                case Dakota::TokenType::COLON: std::cout << "COLON"; break;
                case Dakota::TokenType::ASSIGN: std::cout << "ASSIGN"; break;
                case Dakota::TokenType::INTEGER: std::cout << "INTEGER"; break;
                case Dakota::TokenType::RETURN: std::cout << "RETURN"; break;
                case Dakota::TokenType::EOF_TOKEN: std::cout << "EOF"; break;
                default: std::cout << "OTHER(" << static_cast<int>(tokens[i].type) << ")"; break;
            }
            
            if (!tokens[i].value.empty()) {
                std::cout << " '" << tokens[i].value << "'";
            }
            std::cout << " (line " << tokens[i].line << ")\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    
    return 0;
}
