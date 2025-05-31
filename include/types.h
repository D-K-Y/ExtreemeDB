#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <unordered_map>

namespace InMemoryDB {

// Data types
using Value = std::variant<int, double, std::string, bool>;

enum class DataType {
    INTEGER,
    DOUBLE,
    STRING,
    BOOLEAN
};

// Column definition
struct Column {
    std::string name;
    DataType type;
    bool nullable;
    bool primary_key;
    
    Column(const std::string& n, DataType t, bool null = true, bool pk = false)
        : name(n), type(t), nullable(null), primary_key(pk) {}
};

// Row data
using Row = std::vector<Value>;

// Query result
struct QueryResult {
    std::vector<Column> columns;
    std::vector<Row> rows;
    bool success;
    std::string error_message;
    
    QueryResult() : success(false) {}
};

}

#endif