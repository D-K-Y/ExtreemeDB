#include "plsql_parser.h"
#include "storage_engine.h"
#include <iostream>
#include <sstream>

namespace InMemoryDB {

class PLSQLExecutor {
private:
    StorageEngine* storage_engine_;

public:
    PLSQLExecutor(StorageEngine* engine) : storage_engine_(engine) {}
    
    QueryResult execute(const std::string& sql) {
        try {
            // Tokenize
            PLSQLLexer lexer(sql);
            auto tokens = lexer.tokenize();
            
            // Parse and execute
            PLSQLParser parser(tokens);
            return parser.parse();
            
        } catch (const std::exception& e) {
            QueryResult result;
            result.error_message = e.what();
            return result;
        }
    }
    
    void printResult(const QueryResult& result) {
        if (!result.success) {
            std::cout << "Error: " << result.error_message << std::endl;
            return;
        }
        
        if (result.columns.empty()) {
            std::cout << "Query executed successfully." << std::endl;
            return;
        }
        
        // Print column headers
        for (size_t i = 0; i < result.columns.size(); ++i) {
            std::cout << result.columns[i].name;
            if (i < result.columns.size() - 1) std::cout << "\t";
        }
        std::cout << std::endl;
        
        // Print separator
        for (size_t i = 0; i < result.columns.size(); ++i) {
            std::cout << std::string(result.columns[i].name.length(), '-');
            if (i < result.columns.size() - 1) std::cout << "\t";
        }
        std::cout << std::endl;
        
        // Print rows
        for (const auto& row : result.rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::visit([](const auto& v) { std::cout << v; }, row[i]);
                if (i < row.size() - 1) std::cout << "\t";
            }
            std::cout << std::endl;
        }
        
        std::cout << std::endl << result.rows.size() << " row(s) returned." << std::endl;
    }
};

}