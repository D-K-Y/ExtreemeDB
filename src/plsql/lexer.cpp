#include "plsql_parser.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace InMemoryDB {

PLSQLLexer::PLSQLLexer(const std::string& input) : input_(input), position_(0) {}

std::vector<Token> PLSQLLexer::tokenize() {
    std::vector<Token> tokens;
    
    while (position_ < input_.length()) {
        skipWhitespace();
        
        if (position_ >= input_.length()) break;
        
        char ch = currentChar();
        
        if (ch == ';') {
            tokens.push_back({TokenType::SEMICOLON, ";", position_});
            advance();
        } else if (ch == ',') {
            tokens.push_back({TokenType::COMMA, ",", position_});
            advance();
        } else if (ch == '(') {
            tokens.push_back({TokenType::LPAREN, "(", position_});
            advance();
        } else if (ch == ')') {
            tokens.push_back({TokenType::RPAREN, ")", position_});
            advance();
        } else if (ch == '=') {
            tokens.push_back({TokenType::EQ, "=", position_});
            advance();
        } else if (ch == '\'') {
            tokens.push_back(readString());
        } else if (std::isdigit(ch)) {
            tokens.push_back(readNumber());
        } else if (std::isalpha(ch) || ch == '_') {
            tokens.push_back(readIdentifier());
        } else {
            advance(); // Skip unknown characters
        }
    }
    
    tokens.push_back({TokenType::END_OF_FILE, "", position_});
    return tokens;
}

char PLSQLLexer::currentChar() {
    if (position_ >= input_.length()) return '\0';
    return input_[position_];
}

void PLSQLLexer::advance() {
    position_++;
}

void PLSQLLexer::skipWhitespace() {
    while (position_ < input_.length() && std::isspace(input_[position_])) {
        advance();
    }
}

Token PLSQLLexer::readString() {
    size_t start = position_;
    advance(); // Skip opening quote
    
    std::string value;
    while (position_ < input_.length() && currentChar() != '\'') {
        value += currentChar();
        advance();
    }
    
    if (position_ < input_.length()) {
        advance(); // Skip closing quote
    }
    
    return {TokenType::STRING_LITERAL, value, start};
}

Token PLSQLLexer::readNumber() {
    size_t start = position_;
    std::string value;
    
    while (position_ < input_.length() && (std::isdigit(currentChar()) || currentChar() == '.')) {
        value += currentChar();
        advance();
    }
    
    return {TokenType::NUMBER, value, start};
}

Token PLSQLLexer::readIdentifier() {
    size_t start = position_;
    std::string value;
    
    while (position_ < input_.length() && 
           (std::isalnum(currentChar()) || currentChar() == '_')) {
        value += currentChar();
        advance();
    }
    
    // Convert to uppercase for keyword matching
    std::string upper_value = value;
    std::transform(upper_value.begin(), upper_value.end(), upper_value.begin(), ::toupper);
    
    // Check for keywords
    static std::unordered_map<std::string, TokenType> keywords = {
        {"SELECT", TokenType::SELECT},
        {"INSERT", TokenType::INSERT},
        {"UPDATE", TokenType::UPDATE},
        {"DELETE", TokenType::DELETE},
        {"CREATE", TokenType::CREATE},
        {"DROP", TokenType::DROP},
        {"TABLE", TokenType::TABLE},
        {"FROM", TokenType::FROM},
        {"WHERE", TokenType::WHERE},
        {"INTO", TokenType::INTO},
        {"VALUES", TokenType::VALUES},
        {"SET", TokenType::SET},
        {"AND", TokenType::AND},
        {"OR", TokenType::OR},
        {"NOT", TokenType::NOT}
    };
    
    auto it = keywords.find(upper_value);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    
    return {type, value, start};
}

}// Namespace ends here