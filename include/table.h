#ifndef TABLE_H
#define TABLE_H

#include "types.h"
#include <vector>
#include <memory>
#include <mutex>

namespace InMemoryDB {

class Table {
private:
    std::string name_;
    std::vector<Column> columns_;
    std::vector<Row> rows_;
    mutable std::mutex mutex_;

public:
    Table(const std::string& name, const std::vector<Column>& columns);
    ~Table() = default;

    // Data operations
    bool insert(const Row& row);
    bool update(const std::vector<int>& row_indices, const Row& new_values);
    bool deleteRows(const std::vector<int>& row_indices);
    
    // Query operations
    QueryResult select(const std::vector<std::string>& column_names = {});
    QueryResult selectWhere(const std::string& condition);
    
    // Metadata
    const std::string& getName() const { return name_; }
    const std::vector<Column>& getColumns() const { return columns_; }
    size_t getRowCount() const { return rows_.size(); }
    
    // Index operations
    void createIndex(const std::string& column_name);
    void dropIndex(const std::string& column_name);
};

}

#endif