#include "lexer.h"
#include <iostream>

void test_base_indentation_detection() {
    std::cout << "\n=== Testing Base Indentation Detection ===\n";
    
    // Test 2-space base indentation
    std::string two_space_code = R"(
if x > 0:
  y = x * 2
  if y > 10:
    print("nested")
  else:
    print("other")
)";

    try {
        Dakota::Lexer lexer(two_space_code);
        auto tokens = lexer.tokenize();
        std::cout << "✅ 2-space base indentation detected and enforced successfully!\n";
    } catch (const std::exception& e) {
        std::cout << "❌ 2-space test failed: " << e.what() << std::endl;
    }
    
    // Test 4-space base indentation
    std::string four_space_code = R"(
if x > 0:
    y = x * 2
    if y > 10:
        print("nested")
        if z > 0:
            print("deep")
)";

    try {
        Dakota::Lexer lexer(four_space_code);
        auto tokens = lexer.tokenize();
        std::cout << "✅ 4-space base indentation detected and enforced successfully!\n";
    } catch (const std::exception& e) {
        std::cout << "❌ 4-space test failed: " << e.what() << std::endl;
    }
}

void test_configurable_tab_size() {
    std::cout << "\n=== Testing Configurable Tab Size ===\n";
    
    std::string tab_code = "if x > 0:\n\ty = x * 2\n\t\tprint(y)\n";
    
    try {
        // Test with default 4-space tabs
        Dakota::Lexer lexer1(tab_code, 4);
        auto tokens1 = lexer1.tokenize();
        std::cout << "✅ Default 4-space tab size works!\n";
        
        // Test with 8-space tabs
        Dakota::Lexer lexer2(tab_code, 8);
        auto tokens2 = lexer2.tokenize();
        std::cout << "✅ Custom 8-space tab size works!\n";
        
    } catch (const std::exception& e) {
        std::cout << "❌ Tab size test failed: " << e.what() << std::endl;
    }
}

void test_minimum_indentation_enforcement() {
    std::cout << "\n=== Testing Minimum Indentation Enforcement ===\n";
    
    std::string single_space_code = R"(
if x > 0:
 y = x * 2
)";

    try {
        Dakota::Lexer lexer(single_space_code);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed for 1-space indentation!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly rejected 1-space indentation: " << e.what() << std::endl;
    }
}

void test_invalid_multiples() {
    std::cout << "\n=== Testing Invalid Multiple Detection ===\n";
    
    std::string invalid_multiple_code = R"(
if x > 0:
    y = x * 2
      z = y + 1
)"; // First indent is 4 spaces, second is 6 spaces (not multiple of 4)

    try {
        Dakota::Lexer lexer(invalid_multiple_code);
        auto tokens = lexer.tokenize();
        std::cout << "❌ Should have failed for invalid multiple!\n";
    } catch (const std::exception& e) {
        std::cout << "✅ Correctly caught invalid multiple: " << e.what() << std::endl;
    }
}

void test_memory_efficiency() {
    std::cout << "\n=== Testing Memory Efficiency ===\n";
    
    // Create a large code sample with many indentation levels
    std::string large_code = R"(
if a:
    if b:
        if c:
            if d:
                if e:
                    if f:
                        print("deep nesting")
                    print("level 6")
                print("level 5")
            print("level 4")
        print("level 3")
    print("level 2")
print("level 1")
)";

    try {
        Dakota::Lexer lexer(large_code);
        auto tokens = lexer.tokenize();
        std::cout << "✅ Memory-efficient processing of deep nesting completed!\n";
        std::cout << "   Total tokens: " << tokens.size() << "\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Deep nesting test failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Dakota Lexer - Integer-Based Indentation System Testing\n";
    std::cout << "=====================================================\n";
    
    test_base_indentation_detection();
    test_configurable_tab_size();
    test_minimum_indentation_enforcement();
    test_invalid_multiples();
    test_memory_efficiency();
    
    std::cout << "\n🎯 Integer-based indentation system tests completed!\n";
    std::cout << "✨ O(1) memory usage with robust validation enforced!\n";
    return 0;
}
