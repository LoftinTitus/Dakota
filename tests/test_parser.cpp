#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <chrono>

void test_basic_parsing() {
    std::cout << "\n=== Basic Parsing Test ===\n";
    
    std::string code = R"(
x = 42
y = 3.14
name = "Dakota"
result = x + y * 2
matrix = [1, 2; 3, 4]
product = matrix mult result
)";

    try {
        Dakota::Lexer lexer(code, 4, false); // Skip comments for performance
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokens generated: " << tokens.size() << "\n";
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << " Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << " Parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
            std::cout << "Memory usage: " << parser.get_memory_usage() << " bytes\n";
            
            std::cout << "\nAST Structure:\n";
            parser.print_ast();
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
}

void test_matrix_operations() {
    std::cout << "\n=== Matrix Operations Test ===\n";
    
    std::string code = R"(
A = [1, 0, 0; 0, 1, 0; 0, 0, 1]
B = [2, 1; 1, 2]
C = A mult B
result = C ** 2
)";

    try {
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "  Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "   Matrix parsing successful!\n";
            
            auto matrix_ops = parser.find_matrix_multiplications();
            std::cout << "Matrix multiplications found: " << matrix_ops.size() << "\n";
            
            for (uint32_t node_idx : matrix_ops) {
                std::cout << "  - Matrix mult at node " << node_idx << "\n";
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
}

void test_control_flow() {
    std::cout << "\n=== Control Flow Test ===\n";
    
    std::string code = R"(
if x > 0:
    y = x * 2
    if y > 10:
        result = "large"
    else:
        result = "small"
else:
    result = "negative"

while i < 10:
    i = i + 1
    print(i)
)";

    try {
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "  Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "   Control flow parsing successful!\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
}

void test_function_definition() {
    std::cout << "\n=== Function Definition Test ===\n";
    
    std::string code = R"(
function matrix_multiply(a, b):
    result = a mult b
    return result

function fibonacci(n):
    if n <= 1:
        return n
    else:
        return fibonacci(n - 1) + fibonacci(n - 2)

x = matrix_multiply(A, B)
fib = fibonacci(10)
)";

    try {
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        if (parser.has_error()) {
            std::cout << "  Parse error: " << parser.get_error() << "\n";
        } else {
            std::cout << "   Function parsing successful!\n";
            
            auto func_calls = parser.find_function_calls();
            std::cout << "Function calls found: " << func_calls.size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
}

void benchmark_parser_performance() {
    std::cout << "\n=== Parser Performance Benchmark ===\n";
    
    // Generate large code sample
    std::string large_code;
    for (int i = 0; i < 1000; i++) {
        large_code += "x" + std::to_string(i) + " = matrix" + std::to_string(i) + " mult vector" + std::to_string(i) + "\n";
        large_code += "result" + std::to_string(i) + " = x" + std::to_string(i) + " ** 2.0 + " + std::to_string(i) + "\n";
        if (i % 100 == 0) {
            large_code += "if result" + std::to_string(i) + " > threshold:\n";
            large_code += "    optimized_result = fast_compute(result" + std::to_string(i) + ")\n";
        }
    }
    
    std::cout << "Code size: " << large_code.length() << " bytes\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        Dakota::Lexer lexer(large_code, 4, false);
        auto tokens = lexer.tokenize();
        
        auto lex_end = std::chrono::high_resolution_clock::now();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        auto parse_end = std::chrono::high_resolution_clock::now();
        
        if (parser.has_error()) {
            std::cout << "  Parse error: " << parser.get_error() << "\n";
        } else {
            auto lex_time = std::chrono::duration_cast<std::chrono::microseconds>(lex_end - start);
            auto parse_time = std::chrono::duration_cast<std::chrono::microseconds>(parse_end - lex_end);
            auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(parse_end - start);
            
            std::cout << "   Performance benchmark completed!\n";
            std::cout << "Lexing time: " << lex_time.count() << " μs\n";
            std::cout << "Parsing time: " << parse_time.count() << " μs\n";
            std::cout << "Total time: " << total_time.count() << " μs\n";
            std::cout << "Tokens: " << tokens.size() << "\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
            std::cout << "Memory usage: " << parser.get_memory_usage() << " bytes\n";
            std::cout << "Throughput: " << (large_code.length() / 1024.0) / (total_time.count() / 1000000.0) << " KB/s\n";
            
            // Engineering-specific analysis
            auto matrix_ops = parser.find_matrix_multiplications();
            std::cout << "Matrix operations detected: " << matrix_ops.size() << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Dakota Parser\n";
    test_basic_parsing();
    test_matrix_operations();
    test_control_flow();
    test_function_definition();
    benchmark_parser_performance();
    
    std::cout << "\n Parser testing completed!\n";
    
    return 0;
}
