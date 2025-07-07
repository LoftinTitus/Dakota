#include "lexer.h"
#include <iostream>

void test_good_indentation() {
    std::cout << "\n=== Testing Good Indentation ===\n";
    std::string good_code = R"(
if x > 0:
    y = x * 2
    if y > 10:
        print("Large value")
    else:
        print("Small value")
else:
    print("Negative")
)";

    try {
        Dakota::Lexer lexer(good_code);
        auto tokens = lexer.tokenize();
        std::cout << "✅ Good indentation passed! Found " << tokens.size() << " tokens.\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Unexpected error: " << e.what() << std::endl;
    }
}

void test_mixed_tabs_spaces() {
    std::cout << "\n=== Testing Mixed Tabs and Spaces ===\n";
    std::string mixed_code = "if x > 0:\n    y = x * 2\n\tz = y + 1\n";  // spaces then tab
    
    try {
        Dakota::Lexer lexer(mixed_code);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed but didn't!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly caught mixed indentation: " << e.what() << std::endl;
    }
}

void test_inconsistent_spacing() {
    std::cout << "\n=== Testing Inconsistent Spacing ===\n";
    std::string inconsistent_code = R"(
if x > 0:
    y = x * 2
   z = y + 1
)"; // 4 spaces then 3 spaces

    try {
        Dakota::Lexer lexer(inconsistent_code);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed but didn't!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly caught inconsistent spacing: " << e.what() << std::endl;
    }
}

void test_invalid_dedent() {
    std::cout << "\n=== Testing Invalid Dedent ===\n";
    std::string invalid_dedent = R"(
if x > 0:
    if y > 0:
        print("nested")
  print("invalid dedent")
)"; // dedent to 2 spaces, but original was 4

    try {
        Dakota::Lexer lexer(invalid_dedent);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed but didn't!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly caught invalid dedent: " << e.what() << std::endl;
    }
}

void test_minimal_indentation() {
    std::cout << "\n=== Testing Minimal Indentation (1 space) ===\n";
    std::string minimal_code = R"(
if x > 0:
 y = x * 2
)"; // only 1 space indentation

    try {
        Dakota::Lexer lexer(minimal_code);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed but didn't!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly caught minimal indentation: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Dakota Lexer - Robust Indentation Testing\n";
    std::cout << "==========================================\n";
    
    test_good_indentation();
    test_mixed_tabs_spaces();
    test_inconsistent_spacing();
    test_invalid_dedent();
    test_minimal_indentation();
    
    std::cout << "\nAll indentation tests completed!\n";
    return 0;
}
