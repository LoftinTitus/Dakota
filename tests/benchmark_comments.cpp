#include "lexer.h"
#include <iostream>
#include <chrono>
#include <string>

std::string generate_test_code_with_comments(int lines) {
    std::string code;
    for (int i = 0; i < lines; i++) {
        code += "\\ This is comment line " + std::to_string(i) + " with some text\n";
        code += "x" + std::to_string(i) + " = " + std::to_string(i * 2) + "\n";
    }
    return code;
}

std::string generate_test_code_without_comments(int lines) {
    std::string code;
    for (int i = 0; i < lines; i++) {
        code += "x" + std::to_string(i) + " = " + std::to_string(i * 2) + "\n";
    }
    return code;
}

void benchmark_tokenization(const std::string& description, const std::string& code) {
    std::cout << "\n" << description << "\n";
    std::cout << std::string(description.length(), '=') << "\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Dakota::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Tokens generated: " << tokens.size() << "\n";
    std::cout << "Time taken: " << duration.count() << " microseconds\n";
    std::cout << "Code size: " << code.length() << " bytes\n";
    
    // Count comment tokens (should be 0 since they're filtered out)
    int comment_count = 0;
    for (const auto& token : tokens) {
        if (token.type == Dakota::TokenType::COMMENT) {
            comment_count++;
        }
    }
    std::cout << "Comment tokens in final output: " << comment_count << "\n";
}

int main() {
    std::cout << "Dakota Lexer - Comment Processing Performance Analysis\n";
    std::cout << "====================================================\n";
    
    const int TEST_LINES = 1000;
    
    // Generate test code
    auto code_with_comments = generate_test_code_with_comments(TEST_LINES);
    auto code_without_comments = generate_test_code_without_comments(TEST_LINES);
    
    // Benchmark both scenarios
    benchmark_tokenization("With Comments (Current Implementation)", code_with_comments);
    benchmark_tokenization("Without Comments (Baseline)", code_without_comments);
    
    std::cout << "\n📊 Analysis:\n";
    std::cout << "- Comments are tokenized but then filtered out\n";
    std::cout << "- This wastes CPU cycles and memory allocation\n";
    std::cout << "- Real performance gain would come from skipping comment tokenization entirely\n";
    
    return 0;
}
