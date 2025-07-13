#include "../src/interpreter.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Matrix Operations Verification\n";
    std::cout << "==============================\n";
    
    std::string code = R"(A = [1, 2; 3, 4]
B = [2, 0; 1, 2]
C = A + B
D = A mult B)";
    
    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return 1;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        auto A = env->get("A").as_matrix();
        auto B = env->get("B").as_matrix();
        auto C = env->get("C").as_matrix();
        auto D = env->get("D").as_matrix();
        
        std::cout << "A = " << env->get("A").to_string() << std::endl;
        std::cout << "B = " << env->get("B").to_string() << std::endl;
        std::cout << "C = A + B = " << env->get("C").to_string() << std::endl;
        std::cout << "D = A mult B = " << env->get("D").to_string() << std::endl;
        
        // Verify addition
        assert(C[0][0] == 3.0 && C[0][1] == 2.0);
        assert(C[1][0] == 4.0 && C[1][1] == 6.0);
        
        // Verify multiplication
        assert(D[0][0] == 4.0 && D[0][1] == 4.0);
        assert(D[1][0] == 10.0 && D[1][1] == 8.0);
        
        std::cout << "✓ All matrix operations verified!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
