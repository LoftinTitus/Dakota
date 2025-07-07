#include "lexer.h"
#include <iostream>
#include <chrono>
#include <string>

std::string generate_comment_heavy_code(int lines) {
    std::string code;
    for (int i = 0; i < lines; i++) {
        code += "\\ This is a detailed comment explaining the algorithm step " + std::to_string(i) + "\n";
        code += "\\ with multiple lines of documentation and explanation text\n";
        code += "\\ including performance notes and implementation details here\n";
        code += "x" + std::to_string(i) + " = matrix_multiply(A, B) ** 2.5\n";
        if (i % 10 == 0) {
            code += "if x" + std::to_string(i) + " > threshold:\n";
            code += "    \\ Another comment inside the conditional block\n";
            code += "    result = optimize_computation(x" + std::to_string(i) + ")\n";
        }
    }
    return code;
}

void benchmark_comment_processing(const std::string& description, 
                                 const std::string& code, 
                                 bool preserve_comments) {
    std::cout << "\n" << description << "\n";
    std::cout << std::string(description.length(), '=') << "\n";
    
    // Warm up
    for (int i = 0; i < 3; i++) {
        Dakota::Lexer lexer(code, 4, preserve_comments);
        auto tokens = lexer.tokenize();
    }
    
    // Actual benchmark
    auto start = std::chrono::high_resolution_clock::now();
    
    Dakota::Lexer lexer(code, 4, preserve_comments);
    auto tokens = lexer.tokenize();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Count different token types
    int comment_tokens = 0, code_tokens = 0;
    for (const auto& token : tokens) {
        if (token.type == Dakota::TokenType::COMMENT) {
            comment_tokens++;
        } else if (token.type != Dakota::TokenType::NEWLINE) {
            code_tokens++;
        }
    }
    
    std::cout << "Total tokens: " << tokens.size() << "\n";
    std::cout << "Comment tokens: " << comment_tokens << "\n";
    std::cout << "Code tokens: " << code_tokens << "\n";
    std::cout << "Time taken: " << duration.count() << " microseconds\n";
    std::cout << "Code size: " << code.length() << " bytes\n";
    std::cout << "Speed: " << (code.length() / 1024.0) / (duration.count() / 1000000.0) << " KB/s\n";
}

int main() {
    std::cout << "Dakota Lexer - Optimized Comment Processing Benchmark\n";
    std::cout << "===================================================\n";
    
    const int TEST_LINES = 500;
    auto comment_heavy_code = generate_comment_heavy_code(TEST_LINES);
    
    std::cout << "Testing with " << TEST_LINES << " lines of comment-heavy code...\n";
    
    // Test with comments preserved (old behavior)
    benchmark_comment_processing("🐌 WITH Comment Tokenization (preserve_comments=true)", 
                                comment_heavy_code, true);
    
    // Test with comments skipped (optimized)
    benchmark_comment_processing("🚀 WITHOUT Comment Tokenization (preserve_comments=false)", 
                                comment_heavy_code, false);
    
    std::cout << "\n📈 Performance Analysis:\n";
    std::cout << "- Skipping comments eliminates token creation overhead\n";
    std::cout << "- Reduces memory allocations for comment tokens\n";
    std::cout << "- Faster parser processing (fewer tokens to handle)\n";
    std::cout << "- Smaller token streams for subsequent compilation phases\n";
    std::cout << "\n💡 Recommendation: Use preserve_comments=false for production builds\n";
    std::cout << "   Use preserve_comments=true only for IDE/documentation tools\n";
    
    return 0;
}
