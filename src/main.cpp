#include "storage_engine.h"
#include "plsql_parser.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <memory>

using namespace InMemoryDB;

// Define the global variable INSIDE the namespace - remove the duplicate declaration
namespace InMemoryDB {
    StorageEngine* g_storage_engine = nullptr;
}

void printWelcome() {
    std::cout << "========================================" << std::endl;
    std::cout << "    In-Memory PL/SQL Database v1.0     " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  CREATE TABLE name (col1 type, col2 type, ...);" << std::endl;
    std::cout << "  INSERT INTO name VALUES (val1, val2, ...);" << std::endl;
    std::cout << "  SELECT * FROM name;" << std::endl;
    std::cout << "  SELECT col1, col2 FROM name;" << std::endl;
    std::cout << "  DROP TABLE name;" << std::endl;
    std::cout << "  exit - quit the program" << std::endl;
    std::cout << "========================================" << std::endl;
}

void executeQuery(const std::string& sql) {
    try {
        // Tokenize
        PLSQLLexer lexer(sql);
        auto tokens = lexer.tokenize();
        
        // Parse and execute
        PLSQLParser parser(tokens);
        QueryResult result = parser.parse();
        
        if (result.success) {
            if (!result.columns.empty()) {
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
            } else {
                std::cout << "Query executed successfully." << std::endl;
            }
        } else {
            std::cout << "Error: " << result.error_message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Initialize storage engine
    StorageEngine storage;
    g_storage_engine = &storage;  // This now refers to InMemoryDB::g_storage_engine
    
    printWelcome();
    
    std::string input;
    while (true) {
        std::cout << std::endl << "SQL> ";
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        executeQuery(input);
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}