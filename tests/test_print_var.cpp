#include "../src/interpreter.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>

int main() {
    std::cout << "Print Test with Variable\n";
    std::cout << "========================\n";
    
    std::string code = R"(print("Hello Dakota")
x = 42
print(x))";
    
    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return 1;
        }
        
        std::cout << "AST Structure:\n";
        parser.print_ast(0);
        
        Dakota::Interpreter interpreter(parser);
        std::cout << "\nExpected output:\n";
        interpreter.interpret();
        
        // Check if variable was set
        auto env = interpreter.get_global_environment();
        try {
            auto x_val = env->get("x");
            std::cout << "\nVariable x = " << x_val.to_string() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "\nCouldn't access variable x: " << e.what() << std::endl;
        }
        
        std::cout << "Test completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
