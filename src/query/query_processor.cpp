#ifndef CONFIG_H
#define CONFIG_H

#include "storage_engine.h"
#include "plsql_parser.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

class Config {
public:
    void loadConfig(const std::string& filename);
    std::string getSetting(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> settings;
};

namespace InMemoryDB {

class QueryProcessor {
private:
    StorageEngine* storage_engine_;

public:
    QueryProcessor(StorageEngine* engine) : storage_engine_(engine) {}
    
    QueryResult processQuery(const std::string& sql) {
        // Normalize SQL (trim, convert to uppercase for keywords)
        std::string normalized_sql = normalizeSql(sql);
        
        // Basic query validation
        if (!validateSql(normalized_sql)) {
            QueryResult result;
            result.error_message = "Invalid SQL syntax";
            return result;
        }
        
        // Parse and execute
        PLSQLLexer lexer(normalized_sql);
        auto tokens = lexer.tokenize();
        
        PLSQLParser parser(tokens);
        return parser.parse();
    }
    
private:
    std::string normalizeSql(const std::string& sql) {
        std::string result = sql;
        
        // Trim whitespace
        result.erase(0, result.find_first_not_of(" \t\n\r"));
        result.erase(result.find_last_not_of(" \t\n\r") + 1);
        
        // Ensure semicolon at end if not present
        if (!result.empty() && result.back() != ';') {
            result += ';';
        }
        
        return result;
    }
    
    bool validateSql(const std::string& sql) {
        if (sql.empty()) return false;
        
        // Basic validation - check for balanced parentheses
        int paren_count = 0;
        for (char c : sql) {
            if (c == '(') paren_count++;
            else if (c == ')') paren_count--;
            if (paren_count < 0) return false;
        }
        
        return paren_count == 0;
    }
};

}

#endif // CONFIG_H