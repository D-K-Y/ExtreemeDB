#ifndef QUERIES_H
#define QUERIES_H

#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include "table.h"
#include "storage_engine.h"

// Function to execute a SQL query
bool executeQuery(const std::string& query);

// Function to fetch results from a SQL query
std::vector<std::tuple<int, std::string, bool>> fetchResults(const std::string& query);

namespace InMemoryDB {

Table::Table(const std::string& name, const std::vector<Column>& columns)
    : name_(name), columns_(columns) {
}

bool Table::insert(const Row& row) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (row.size() != columns_.size()) {
        return false; // Column count mismatch
    }
    
    // Validate data types and constraints
    for (size_t i = 0; i < row.size(); ++i) {
        // Type validation would go here
        if (!columns_[i].nullable && std::holds_alternative<std::string>(row[i]) && 
            std::get<std::string>(row[i]).empty()) {
            return false; // NULL constraint violation
        }
    }
    
    rows_.push_back(row);
    return true;
}

bool Table::update(const std::vector<int>& row_indices, const Row& new_values) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (int index : row_indices) {
        if (index >= 0 && index < static_cast<int>(rows_.size())) {
            // Update specific columns based on new_values
            for (size_t i = 0; i < new_values.size() && i < rows_[index].size(); ++i) {
                rows_[index][i] = new_values[i];
            }
        }
    }
    
    return true;
}

bool Table::deleteRows(const std::vector<int>& row_indices) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Sort indices in descending order to avoid index shifting issues
    std::vector<int> sorted_indices = row_indices;
    std::sort(sorted_indices.rbegin(), sorted_indices.rend());
    
    for (int index : sorted_indices) {
        if (index >= 0 && index < static_cast<int>(rows_.size())) {
            rows_.erase(rows_.begin() + index);
        }
    }
    
    return true;
}

QueryResult Table::select(const std::vector<std::string>& column_names) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    QueryResult result;
    result.success = true;
    
    if (column_names.empty()) {
        // Select all columns
        result.columns = columns_;
        result.rows = rows_;
    } else {
        // Select specific columns
        std::vector<int> column_indices;
        for (const std::string& col_name : column_names) {
            for (size_t i = 0; i < columns_.size(); ++i) {
                if (columns_[i].name == col_name) {
                    column_indices.push_back(i);
                    result.columns.push_back(columns_[i]);
                    break;
                }
            }
        }
        
        // Extract specific columns from each row
        for (const Row& row : rows_) {
            Row filtered_row;
            for (int index : column_indices) {
                filtered_row.push_back(row[index]);
            }
            result.rows.push_back(filtered_row);
        }
    }
    
    return result;
}

QueryResult Table::selectWhere(const std::string& condition) {
    // Simple WHERE clause implementation
    // In a real implementation, this would parse and evaluate conditions
    return select(); // For now, return all rows
}

void Table::createIndex(const std::string& column_name) {
    // Index creation implementation would go here
}

void Table::dropIndex(const std::string& column_name) {
    // Index dropping implementation would go here
}

}

#endif // QUERIES_H