#include "lexer.h"
#include <iostream>

int main() {
    // Example Dakota code to tokenize
    std::string dakota_code = R"(
\ This is a comment
x = [1; 2; 3]
A = [1, 0, 0; 0, 1, 0; 0, 0, 1]
y = A mult x

if x[0] > 0:
    result = x ** 2.5
    print("Positive result:", result)
else:
    result = -x
    
function multiply_matrix(a, b):
    return a mult b
)";

    try {
        Dakota::Lexer lexer(dakota_code);
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokenization complete! Found " << tokens.size() << " tokens:\n\n";
        lexer.print_tokens(tokens);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
