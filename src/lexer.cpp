#include "lexer.h"
#include <iostream>
#include <cctype>
#include <stdexcept>

namespace Dakota {

// Keywords mapping
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"elif", TokenType::ELIF},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"function", TokenType::FUNCTION},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"mult", TokenType::MATMUL}
};

Lexer::Lexer(const std::string& source_code) 
    : source(source_code), position(0), line(1), column(1) {
    indent_stack.push_back(0); // Start with no indentation
    current_char = position < source.length() ? source[position] : '\0';
}

void Lexer::advance() {
    if (current_char == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    
    position++;
    current_char = position < source.length() ? source[position] : '\0';
}

char Lexer::peek(size_t offset) const {
    size_t peek_pos = position + offset;
    return peek_pos < source.length() ? source[peek_pos] : '\0';
}

void Lexer::skip_whitespace() {
    while (current_char == ' ' || current_char == '\t' || current_char == '\r') {
        advance();
    }
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(c);
}

bool Lexer::is_alnum(char c) const {
    return std::isalnum(c) || c == '_';
}

Token Lexer::make_number() {
    size_t start_line = line;
    size_t start_column = column;
    std::string number_str;
    bool is_float = false;
    
    // Handle integer part
    while (is_digit(current_char)) {
        number_str += current_char;
        advance();
    }
    
    // Handle decimal point
    if (current_char == '.' && is_digit(peek())) {
        is_float = true;
        number_str += current_char;
        advance();
        
        while (is_digit(current_char)) {
            number_str += current_char;
            advance();
        }
    }
    
    // Handle scientific notation
    if (current_char == 'e' || current_char == 'E') {
        is_float = true;
        number_str += current_char;
        advance();
        
        if (current_char == '+' || current_char == '-') {
            number_str += current_char;
            advance();
        }
        
        while (is_digit(current_char)) {
            number_str += current_char;
            advance();
        }
    }
    
    TokenType type = is_float ? TokenType::FLOAT : TokenType::INTEGER;
    return Token(type, number_str, start_line, start_column);
}

Token Lexer::make_string() {
    size_t start_line = line;
    size_t start_column = column;
    std::string string_value;
    
    advance(); // Skip opening quote
    
    while (current_char != '"' && current_char != '\0') {
        if (current_char == '\\') {
            advance();
            switch (current_char) {
                case 'n': string_value += '\n'; break;
                case 't': string_value += '\t'; break;
                case 'r': string_value += '\r'; break;
                case '\\': string_value += '\\'; break;
                case '"': string_value += '"'; break;
                default: string_value += current_char; break;
            }
        } else {
            string_value += current_char;
        }
        advance();
    }
    
    if (current_char == '"') {
        advance(); // Skip closing quote
    } else {
        throw std::runtime_error("Unterminated string literal at line " + std::to_string(start_line));
    }
    
    return Token(TokenType::STRING, string_value, start_line, start_column);
}

Token Lexer::make_identifier() {
    size_t start_line = line;
    size_t start_column = column;
    std::string identifier;
    
    while (is_alnum(current_char)) {
        identifier += current_char;
        advance();
    }
    
    // Check if it's a keyword
    auto keyword_it = keywords.find(identifier);
    TokenType type = (keyword_it != keywords.end()) ? keyword_it->second : TokenType::IDENTIFIER;
    
    return Token(type, identifier, start_line, start_column);
}

Token Lexer::make_comment() {
    size_t start_line = line;
    size_t start_column = column;
    std::string comment;
    
    advance(); // Skip the backslash
    
    while (current_char != '\n' && current_char != '\0') {
        comment += current_char;
        advance();
    }
    
    return Token(TokenType::COMMENT, comment, start_line, start_column);
}

std::vector<Token> Lexer::handle_indentation() {
    std::vector<Token> tokens;
    int current_indent = 0;
    
    // Count spaces at beginning of line
    while (current_char == ' ' || current_char == '\t') {
        if (current_char == ' ') {
            current_indent++;
        } else if (current_char == '\t') {
            current_indent += 4; // Treat tab as 4 spaces
        }
        advance();
    }
    
    // Skip empty lines (but not comment-only lines - let them be tokenized)
    if (current_char == '\n') {
        return tokens;
    }
    
    int previous_indent = indent_stack.back();
    
    if (current_indent > previous_indent) {
        // Increased indentation
        indent_stack.push_back(current_indent);
        tokens.push_back(Token(TokenType::INDENT, "", line, column));
    } else if (current_indent < previous_indent) {
        // Decreased indentation - may need multiple DEDENT tokens
        while (!indent_stack.empty() && indent_stack.back() > current_indent) {
            indent_stack.pop_back();
            tokens.push_back(Token(TokenType::DEDENT, "", line, column));
        }
        
        if (indent_stack.empty() || indent_stack.back() != current_indent) {
            throw std::runtime_error("Indentation error at line " + std::to_string(line));
        }
    }
    
    return tokens;
}

Token Lexer::next_token() {
    while (current_char != '\0') {
        size_t start_line = line;
        size_t start_column = column;
        
        // Handle newlines and indentation
        if (current_char == '\n') {
            advance();
            return Token(TokenType::NEWLINE, "\n", start_line, start_column);
        }
        
        // Skip whitespace (except newlines)
        if (current_char == ' ' || current_char == '\t' || current_char == '\r') {
            skip_whitespace();
            continue;
        }
        
        // Numbers
        if (is_digit(current_char)) {
            return make_number();
        }
        
        // Strings
        if (current_char == '"') {
            return make_string();
        }
        
        // Identifiers and keywords
        if (is_alpha(current_char)) {
            return make_identifier();
        }
        
        // Comments
        if (current_char == '\\') {
            return make_comment();
        }
        
        // Two-character operators
        if (current_char == '=' && peek() == '=') {
            advance(); advance();
            return Token(TokenType::EQUAL, "==", start_line, start_column);
        }
        if (current_char == '!' && peek() == '=') {
            advance(); advance();
            return Token(TokenType::NOT_EQUAL, "!=", start_line, start_column);
        }
        if (current_char == '<' && peek() == '=') {
            advance(); advance();
            return Token(TokenType::LESS_EQUAL, "<=", start_line, start_column);
        }
        if (current_char == '>' && peek() == '=') {
            advance(); advance();
            return Token(TokenType::GREATER_EQUAL, ">=", start_line, start_column);
        }
        if (current_char == '*' && peek() == '*') {
            advance(); advance();
            return Token(TokenType::POWER, "**", start_line, start_column);
        }
        
        // Single-character tokens
        switch (current_char) {
            case '+': advance(); return Token(TokenType::PLUS, "+", start_line, start_column);
            case '-': advance(); return Token(TokenType::MINUS, "-", start_line, start_column);
            case '*': advance(); return Token(TokenType::MULTIPLY, "*", start_line, start_column);
            case '/': advance(); return Token(TokenType::DIVIDE, "/", start_line, start_column);
            case '=': advance(); return Token(TokenType::ASSIGN, "=", start_line, start_column);
            case '<': advance(); return Token(TokenType::LESS, "<", start_line, start_column);
            case '>': advance(); return Token(TokenType::GREATER, ">", start_line, start_column);
            case '(': advance(); return Token(TokenType::LPAREN, "(", start_line, start_column);
            case ')': advance(); return Token(TokenType::RPAREN, ")", start_line, start_column);
            case '[': advance(); return Token(TokenType::LBRACKET, "[", start_line, start_column);
            case ']': advance(); return Token(TokenType::RBRACKET, "]", start_line, start_column);
            case '{': advance(); return Token(TokenType::LBRACE, "{", start_line, start_column);
            case '}': advance(); return Token(TokenType::RBRACE, "}", start_line, start_column);
            case ',': advance(); return Token(TokenType::COMMA, ",", start_line, start_column);
            case ';': advance(); return Token(TokenType::SEMICOLON, ";", start_line, start_column);
            case ':': advance(); return Token(TokenType::COLON, ":", start_line, start_column);
            case '.': advance(); return Token(TokenType::DOT, ".", start_line, start_column);
            
            default:
                advance();
                return Token(TokenType::INVALID, std::string(1, source[position-1]), start_line, start_column);
        }
    }
    
    return Token(TokenType::EOF_TOKEN, "", line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    bool at_line_start = true;
    
    while (current_char != '\0') {
        // Handle indentation at the start of lines
        if (at_line_start && (current_char == ' ' || current_char == '\t')) {
            auto indent_tokens = handle_indentation();
            tokens.insert(tokens.end(), indent_tokens.begin(), indent_tokens.end());
            at_line_start = false;
            continue;
        }
        
        Token token = next_token();
        
        if (token.type == TokenType::NEWLINE) {
            at_line_start = true;
        } else {
            at_line_start = false;
        }
        
        // Skip comments in the final token stream
        if (token.type != TokenType::COMMENT) {
            tokens.push_back(token);
        }
        
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    
    // Add final DEDENT tokens if needed
    while (indent_stack.size() > 1) {
        indent_stack.pop_back();
        tokens.push_back(Token(TokenType::DEDENT, "", line, column));
    }
    
    return tokens;
}

std::string Lexer::token_type_to_string(TokenType type) const {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::BOOLEAN: return "BOOLEAN";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::ELIF: return "ELIF";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::IN: return "IN";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::RETURN: return "RETURN";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::POWER: return "POWER";
        case TokenType::MATMUL: return "MATMUL";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::DOT: return "DOT";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

void Lexer::print_tokens(const std::vector<Token>& tokens) const {
    for (const auto& token : tokens) {
        std::cout << token_type_to_string(token.type) << " ";
        if (!token.value.empty()) {
            std::cout << "'" << token.value << "' ";
        }
        std::cout << "(" << token.line << ":" << token.column << ")" << std::endl;
    }
}

} // namespace Dakota

