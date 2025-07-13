#include "../src/interpreter.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>

int main() {
    std::cout << "Matrix Test\n";
    std::cout << "===========\n";
    
    std::string code = "A = [1, 2; 3, 4]";
    
    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokens: " << tokens.size() << std::endl;
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return 1;
        }
        
        std::cout << "Parse successful!" << std::endl;
        
        // Debug: Check matrix literal node structure
        const auto& nodes = parser.get_nodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].type == Dakota::NodeType::MATRIX_LITERAL) {
                std::cout << "MATRIX_LITERAL node at index " << i << ":\n";
                std::cout << "  rows: " << nodes[i].matrix_literal.rows << "\n";
                std::cout << "  cols: " << nodes[i].matrix_literal.cols << "\n";
                std::cout << "  elements_start_index: " << nodes[i].matrix_literal.elements_start_index << "\n";
                std::cout << "  is_empty: " << (nodes[i].matrix_literal.is_empty ? "true" : "false") << "\n";
                std::cout << "  first_child_index: " << nodes[i].first_child_index << "\n";
                std::cout << "  next_sibling_index: " << nodes[i].next_sibling_index << "\n";
            }
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        try {
            auto A = env->get("A");
            std::cout << "A = " << A.to_string() << std::endl;
            std::cout << "Success!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error accessing A: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
