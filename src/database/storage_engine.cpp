#include "storage_engine.h"
#include <algorithm>

namespace InMemoryDB {

bool StorageEngine::createTable(const std::string& name, const std::vector<Column>& columns) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (tables_.find(name) != tables_.end()) {
        return false; // Table already exists
    }
    
    tables_[name] = std::make_unique<Table>(name, columns);
    return true;
}

bool StorageEngine::dropTable(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return false; // Table doesn't exist
    }
    
    tables_.erase(it);
    return true;
}

Table* StorageEngine::getTable(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

std::vector<std::string> StorageEngine::getTableNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> names;
    for (const auto& pair : tables_) {
        names.push_back(pair.first);
    }
    
    return names;
}

void StorageEngine::beginTransaction() {
    // Transaction implementation would go here
}

void StorageEngine::commit() {
    // Commit implementation would go here
}

void StorageEngine::rollback() {
    // Rollback implementation would go here
}

}