#include "../src/interpreter.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>

int main() {
    std::cout << "Simple Dakota Interpreter Test\n";
    std::cout << "==============================\n";
    
    std::string code = R"(x = 10
y = 5
sum = x + y)";
    
    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokens: " << tokens.size() << std::endl;
        lexer.print_tokens(tokens);
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return 1;
        }
        
        std::cout << "Parse successful! AST nodes: " << parser.get_nodes().size() << std::endl;
        parser.print_ast(0);
        
        // Debug: Check the first node details
        const auto& nodes = parser.get_nodes();
        if (!nodes.empty()) {
            std::cout << "Root node type: " << static_cast<int>(nodes[0].type) << std::endl;
            std::cout << "Root node first_child_index: " << nodes[0].first_child_index << std::endl;
            std::cout << "Root node next_sibling_index: " << nodes[0].next_sibling_index << std::endl;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        try {
            auto x = env->get("x");
            auto y = env->get("y");
            auto sum = env->get("sum");
            std::cout << "x = " << x.to_string() << std::endl;
            std::cout << "y = " << y.to_string() << std::endl;
            std::cout << "sum = " << sum.to_string() << std::endl;
            std::cout << "Success!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error accessing variables: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
