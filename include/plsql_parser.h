#ifndef PLSQL_PARSER_H
#define PLSQL_PARSER_H

#include "types.h"
#include <string>
#include <vector>

namespace InMemoryDB {

enum class TokenType {
    SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, TABLE,
    FROM, WHERE, INTO, VALUES, SET,
    IDENTIFIER, NUMBER, STRING_LITERAL,
    SEMICOLON, COMMA, LPAREN, RPAREN,
    EQ, NE, LT, GT, LE, GE,
    AND, OR, NOT,
    END_OF_FILE, INVALID
};

struct Token {
    TokenType type;
    std::string value;
    size_t position;
};

class PLSQLLexer {
private:
    std::string input_;
    size_t position_;
    
public:
    PLSQLLexer(const std::string& input);
    std::vector<Token> tokenize();
    
private:
    char currentChar();
    void advance();
    void skipWhitespace();
    Token readString();
    Token readNumber();
    Token readIdentifier();
};

class PLSQLParser {
private:
    std::vector<Token> tokens_;
    size_t current_;
    
public:
    PLSQLParser(const std::vector<Token>& tokens);
    QueryResult parse();
    
private:
    Token currentToken();
    void advance();
    bool match(TokenType type);
    QueryResult parseSelect();
    QueryResult parseInsert();
    QueryResult parseUpdate();
    QueryResult parseDelete();
    QueryResult parseCreate();
    QueryResult parseDrop();
};

}

#endif