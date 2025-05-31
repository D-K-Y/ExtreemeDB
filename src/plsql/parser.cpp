#include "plsql_parser.h"
#include "storage_engine.h"
#include "globals.h"
#include <stdexcept>
#include <algorithm>

namespace InMemoryDB {

PLSQLParser::PLSQLParser(const std::vector<Token>& tokens) : tokens_(tokens), current_(0) {}

QueryResult PLSQLParser::parse() {
    QueryResult result;
    
    if (tokens_.empty() || tokens_[0].type == TokenType::END_OF_FILE) {
        result.error_message = "Empty query";
        return result;
    }
    
    switch (tokens_[0].type) {
        case TokenType::SELECT:
            return parseSelect();
        case TokenType::INSERT:
            return parseInsert();
        case TokenType::UPDATE:
            return parseUpdate();
        case TokenType::DELETE:
            return parseDelete();
        case TokenType::CREATE:
            return parseCreate();
        case TokenType::DROP:
            return parseDrop();
        default:
            result.error_message = "Unsupported SQL statement";
            return result;
    }
}

Token PLSQLParser::currentToken() {
    if (current_ >= tokens_.size()) {
        return {TokenType::END_OF_FILE, "", 0};
    }
    return tokens_[current_];
}

void PLSQLParser::advance() {
    if (current_ < tokens_.size()) {
        current_++;
    }
}

bool PLSQLParser::match(TokenType type) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    return false;
}

QueryResult PLSQLParser::parseSelect() {
    QueryResult result;
    advance(); // consume SELECT
    
    std::vector<std::string> columns;
    
    // Parse column list
    if (currentToken().value == "*") {
        advance();
        // Select all columns - will be handled in table.select()
    } else {
        // Parse specific columns
        do {
            if (currentToken().type == TokenType::IDENTIFIER) {
                columns.push_back(currentToken().value);
                advance();
            } else {
                result.error_message = "Expected column name";
                return result;
            }
        } while (match(TokenType::COMMA));
    }
    
    // Parse FROM clause
    if (!match(TokenType::FROM)) {
        result.error_message = "Expected FROM keyword";
        return result;
    }
    
    if (currentToken().type != TokenType::IDENTIFIER) {
        result.error_message = "Expected table name";
        return result;
    }
    
    std::string table_name = currentToken().value;
    advance();
    
    // Get table from storage engine
    if (!g_storage_engine) {
        result.error_message = "Storage engine not initialized";
        return result;
    }
    
    Table* table = g_storage_engine->getTable(table_name);
    if (!table) {
        result.error_message = "Table '" + table_name + "' does not exist";
        return result;
    }
    
    // Execute select
    if (columns.empty()) {
        result = table->select(); // Select all
    } else {
        result = table->select(columns);
    }
    
    return result;
}

QueryResult PLSQLParser::parseInsert() {
    QueryResult result;
    advance(); // consume INSERT
    
    if (!match(TokenType::INTO)) {
        result.error_message = "Expected INTO keyword";
        return result;
    }
    
    if (currentToken().type != TokenType::IDENTIFIER) {
        result.error_message = "Expected table name";
        return result;
    }
    
    std::string table_name = currentToken().value;
    advance();
    
    if (!match(TokenType::VALUES)) {
        result.error_message = "Expected VALUES keyword";
        return result;
    }
    
    if (!match(TokenType::LPAREN)) {
        result.error_message = "Expected '('";
        return result;
    }
    
    // Parse values
    Row values;
    do {
        Token token = currentToken();
        if (token.type == TokenType::NUMBER) {
            // Try to parse as integer first, then double
            std::string val = token.value;
            if (val.find('.') != std::string::npos) {
                values.push_back(std::stod(val));
            } else {
                values.push_back(std::stoi(val));
            }
            advance();
        } else if (token.type == TokenType::STRING_LITERAL) {
            values.push_back(token.value);
            advance();
        } else if (token.type == TokenType::IDENTIFIER) {
            // Handle boolean values or null
            std::string val = token.value;
            std::transform(val.begin(), val.end(), val.begin(), ::toupper);
            if (val == "TRUE") {
                values.push_back(true);
            } else if (val == "FALSE") {
                values.push_back(false);
            } else {
                values.push_back(token.value); // Treat as string
            }
            advance();
        } else {
            result.error_message = "Invalid value type";
            return result;
        }
    } while (match(TokenType::COMMA));
    
    if (!match(TokenType::RPAREN)) {
        result.error_message = "Expected ')'";
        return result;
    }
    
    // Get table and insert
    if (!g_storage_engine) {
        result.error_message = "Storage engine not initialized";
        return result;
    }
    
    Table* table = g_storage_engine->getTable(table_name);
    if (!table) {
        result.error_message = "Table '" + table_name + "' does not exist";
        return result;
    }
    
    if (table->insert(values)) {
        result.success = true;
    } else {
        result.error_message = "Failed to insert row";
    }
    
    return result;
}

QueryResult PLSQLParser::parseUpdate() {
    QueryResult result;
    result.error_message = "UPDATE not implemented yet";
    return result;
}

QueryResult PLSQLParser::parseDelete() {
    QueryResult result;
    result.error_message = "DELETE not implemented yet";
    return result;
}

QueryResult PLSQLParser::parseCreate() {
    QueryResult result;
    advance(); // consume CREATE
    
    if (!match(TokenType::TABLE)) {
        result.error_message = "Expected TABLE keyword";
        return result;
    }
    
    if (currentToken().type != TokenType::IDENTIFIER) {
        result.error_message = "Expected table name";
        return result;
    }
    
    std::string table_name = currentToken().value;
    advance();
    
    if (!match(TokenType::LPAREN)) {
        result.error_message = "Expected '('";
        return result;
    }
    
    // Parse column definitions
    std::vector<Column> columns;
    do {
        if (currentToken().type != TokenType::IDENTIFIER) {
            result.error_message = "Expected column name";
            return result;
        }
        
        std::string col_name = currentToken().value;
        advance();
        
        if (currentToken().type != TokenType::IDENTIFIER) {
            result.error_message = "Expected column type";
            return result;
        }
        
        std::string type_str = currentToken().value;
        std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::toupper);
        advance();
        
        DataType type;
        if (type_str == "INT" || type_str == "INTEGER") {
            type = DataType::INTEGER;
        } else if (type_str == "DOUBLE" || type_str == "FLOAT") {
            type = DataType::DOUBLE;
        } else if (type_str == "VARCHAR" || type_str == "STRING" || type_str == "TEXT") {
            type = DataType::STRING;
        } else if (type_str == "BOOLEAN" || type_str == "BOOL") {
            type = DataType::BOOLEAN;
        } else {
            result.error_message = "Unsupported column type: " + type_str;
            return result;
        }
        
        columns.emplace_back(col_name, type);
        
    } while (match(TokenType::COMMA));
    
    if (!match(TokenType::RPAREN)) {
        result.error_message = "Expected ')'";
        return result;
    }
    
    // Create table
    if (!g_storage_engine) {
        result.error_message = "Storage engine not initialized";
        return result;
    }
    
    if (g_storage_engine->createTable(table_name, columns)) {
        result.success = true;
    } else {
        result.error_message = "Failed to create table (may already exist)";
    }
    
    return result;
}

QueryResult PLSQLParser::parseDrop() {
    QueryResult result;
    advance(); // consume DROP
    
    if (!match(TokenType::TABLE)) {
        result.error_message = "Expected TABLE keyword";
        return result;
    }
    
    if (currentToken().type != TokenType::IDENTIFIER) {
        result.error_message = "Expected table name";
        return result;
    }
    
    std::string table_name = currentToken().value;
    advance();
    
    // Drop table
    if (!g_storage_engine) {
        result.error_message = "Storage engine not initialized";
        return result;
    }
    
    if (g_storage_engine->dropTable(table_name)) {
        result.success = true;
    } else {
        result.error_message = "Failed to drop table (may not exist)";
    }
    
    return result;
}

}