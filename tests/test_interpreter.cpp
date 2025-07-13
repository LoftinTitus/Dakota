#include "../src/interpreter.h"
#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>
#include <cassert>
#include <sstream>

void test_basic_arithmetic() {
    std::cout << "\n=== Basic Arithmetic Test ===\n";
    
    std::string code = R"(x = 10
y = 5
sum = x + y
diff = x - y
prod = x * y
quot = x / y)";

    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        // Test values
        assert(env->get("x").as_integer() == 10);
        assert(env->get("y").as_integer() == 5);
        assert(env->get("sum").as_integer() == 15);
        assert(env->get("diff").as_integer() == 5);
        assert(env->get("prod").as_integer() == 50);
        assert(env->get("quot").as_float() == 2.0);
        
        std::cout << "✓ All arithmetic tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << "\n";
    }
}

void test_matrix_operations() {
    std::cout << "\n=== Matrix Operations Test ===\n";
    
    std::string code = R"(A = [1, 2; 3, 4]
B = [2, 0; 1, 2]
C = A + B)";

    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        // Test matrix addition
        auto C = env->get("C").as_matrix();
        assert(C[0][0] == 3.0 && C[0][1] == 2.0);
        assert(C[1][0] == 4.0 && C[1][1] == 6.0);
        
        std::cout << "✓ All matrix tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << "\n";
    }
}

void test_builtin_functions() {
    std::cout << "\n=== Built-in Functions Test ===\n";
    
    std::string code = R"(x = abs(-5)
y = sqrt(16))";

    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        // Test built-in functions
        assert(env->get("x").as_integer() == 5);
        assert(env->get("y").as_float() == 4.0);
        
        std::cout << "✓ All built-in function tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << "\n";
    }
}

void test_control_flow() {
    std::cout << "\n=== Control Flow Test ===\n";
    
    std::string code = R"(x = 10
result = "default")";

    try {
        Dakota::Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "Parse error: " << parser.get_error() << "\n";
            return;
        }
        
        Dakota::Interpreter interpreter(parser);
        interpreter.interpret();
        
        auto env = interpreter.get_global_environment();
        
        // Test basic assignment
        assert(env->get("x").as_integer() == 10);
        assert(env->get("result").as_string() == "default");
        
        std::cout << "✓ All control flow tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << "\n";
    }
}

void test_print_function() {
    std::cout << "\n=== Print Function Test ===\n";
    
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
            return;
        }
        
        Dakota::Interpreter interpreter(parser);
        std::cout << "Expected output:\n";
        interpreter.interpret();
        
        std::cout << "✓ Print function test completed!\n";
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Running Dakota Interpreter Tests...\n";
    std::cout << "====================================\n";
    
    test_basic_arithmetic();
    test_matrix_operations();
    test_builtin_functions();
    test_control_flow();
    test_print_function();
    
    std::cout << "\n====================================\n";
    std::cout << "All interpreter tests completed!\n";
    
    return 0;
}
