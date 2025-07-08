#include "../src/parser.h"
#include "../src/lexer.h"
#include <iostream>
#include <vector>
#include <string>

// Test framework
struct TestResult {
    bool passed;
    std::string message;
};

TestResult test_matrix_code(const std::string& code, const std::string& test_name, bool should_succeed = true) {
    std::cout << "\n--- " << test_name << " ---\n";
    std::cout << "Code: " << code << "\n";
    
    try {
        Dakota::Lexer lexer(code, 4, false);
        auto tokens = lexer.tokenize();
        
        Dakota::Parser parser(tokens);
        parser.parse();
        
        bool has_error = parser.has_error();
        
        if (should_succeed && !has_error) {
            std::cout << "✅ PASS: " << test_name << "\n";
            std::cout << "AST nodes: " << parser.get_nodes().size() << "\n";
            return {true, "Parsing successful"};
        } else if (!should_succeed && has_error) {
            std::cout << "✅ PASS: " << test_name << " (expected error)\n";
            std::cout << "Error: " << parser.get_error() << "\n";
            return {true, "Expected error occurred"};
        } else if (should_succeed && has_error) {
            std::cout << "❌ FAIL: " << test_name << "\n";
            std::cout << "Unexpected error: " << parser.get_error() << "\n";
            return {false, "Unexpected parse error"};
        } else {
            std::cout << "❌ FAIL: " << test_name << "\n";
            std::cout << "Expected error but parsing succeeded\n";
            return {false, "Expected error but parsing succeeded"};
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ FAIL: " << test_name << "\n";
        std::cout << "Exception: " << e.what() << "\n";
        return {false, std::string("Exception: ") + e.what()};
    }
}

int main() {
    std::cout << "Dakota Matrix Parsing Test Suite\n";
    std::cout << "=================================\n";
    
    std::vector<TestResult> results;
    
    // Basic matrix tests
    results.push_back(test_matrix_code("[]", "Empty Matrix"));
    results.push_back(test_matrix_code("[1]", "Single Element Matrix"));
    results.push_back(test_matrix_code("[1, 2]", "Single Row Matrix"));
    results.push_back(test_matrix_code("[1; 2]", "Single Column Matrix"));
    results.push_back(test_matrix_code("[1, 2; 3, 4]", "2x2 Matrix"));
    results.push_back(test_matrix_code("[1, 2, 3; 4, 5, 6; 7, 8, 9]", "3x3 Matrix"));
    
    // Matrix with different data types
    results.push_back(test_matrix_code("[1.5, 2.7; 3.14, 4.0]", "Float Matrix"));
    results.push_back(test_matrix_code("[1+2, 3*4; 5-1, 6/2]", "Matrix with Expressions"));
    
    // Assignment tests
    results.push_back(test_matrix_code("x = [1, 2]", "Matrix Assignment"));
    results.push_back(test_matrix_code("matrix = [1, 2; 3, 4]", "2x2 Matrix Assignment"));
    
    // Error cases
    results.push_back(test_matrix_code("[1, 2; 3, 4, 5]", "Inconsistent Row Lengths", false));
    results.push_back(test_matrix_code("[1, 2; 3, 4", "Missing Closing Bracket", false));
    
    // Complex cases
    results.push_back(test_matrix_code("[1, 2] mult [3; 4]", "Matrix Multiplication"));
    
    // Count results
    int passed = 0;
    int total = results.size();
    
    for (const auto& result : results) {
        if (result.passed) passed++;
    }
    
    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << passed << "/" << total << " tests\n";
    
    if (passed == total) {
        std::cout << "🎉 All tests passed!\n";
        return 0;
    } else {
        std::cout << "⚠️  Some tests failed.\n";
        return 1;
    }
}
