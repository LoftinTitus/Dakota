#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

namespace Dakota {

// Token types for Dakota language
enum class TokenType {
    // Literals
    INTEGER,        // 42, 123
    FLOAT,          // 3.14, 2.5e-3
    STRING,         // "hello"
    BOOLEAN,        // true, false
    
    // Identifiers and Keywords
    IDENTIFIER,     // variable names, function names
    
    // Keywords
    IF,             // if
    ELSE,           // else
    ELIF,           // elif
    WHILE,          // while
    FOR,            // for
    IN,             // in
    FUNCTION,       // function
    RETURN,         // return
    TRUE,           // true
    FALSE,          // false
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    POWER,          // **
    MODULO,         // %
    MATMUL,         // mult (matrix multiplication)
    
    // Assignment
    ASSIGN,         // =
    
    // Comparison
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS,           // <
    LESS_EQUAL,     // <=
    GREATER,        // >
    GREATER_EQUAL,  // >=
    
    // Logical
    AND,            // and
    OR,             // or
    NOT,            // not
    
    // Delimiters
    LPAREN,         // (
    RPAREN,         // )
    LBRACKET,       // [
    RBRACKET,       // ]
    LBRACE,         // {
    RBRACE,         // }
    
    // Punctuation
    COMMA,          // ,
    SEMICOLON,      // ; (for matrix row separation)
    COLON,          // :
    DOT,            // .
    
    // Special
    NEWLINE,        // \n (significant for indentation)
    INDENT,         // Increased indentation
    DEDENT,         // Decreased indentation
    COMMENT,        // \ comment
    
    // End of file
    EOF_TOKEN,
    
    // Error token
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    Token(TokenType t, const std::string& v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
private:
    std::string source;
    size_t position;
    size_t line;
    size_t column;
    char current_char;
    
    // Indentation stack for tracking nested blocks
    std::vector<int> indent_stack;
    
    // Indentation style tracking
    enum class IndentStyle { UNKNOWN, SPACES, TABS };
    IndentStyle indent_style;
    int base_indent;        // Base indentation level (detected from first indent)
    int tab_size;           // How many spaces a tab represents
    bool first_indent_detected;
    
    // Keywords map
    static const std::unordered_map<std::string, TokenType> keywords;
    
    // Helper methods
    void advance();
    char peek(size_t offset = 1) const;
    void skip_whitespace();
    Token make_number();
    Token make_string();
    Token make_identifier();
    Token make_comment();
    std::vector<Token> handle_indentation();
    void validate_indentation_style(bool has_spaces, bool has_tabs, int current_indent);
    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alnum(char c) const;
    
public:
    Lexer(const std::string& source_code, int tab_size = 4);
    
    // Main tokenization method
    std::vector<Token> tokenize();
    
    // Get next token (for streaming tokenization)
    Token next_token();
    
    // Utility methods
    std::string token_type_to_string(TokenType type) const;
    void print_tokens(const std::vector<Token>& tokens) const;
};

} // namespace Dakota

#endif // LEXER_H
