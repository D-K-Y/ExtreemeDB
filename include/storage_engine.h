#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include "types.h"
#include "table.h"
#include <unordered_map>
#include <memory>
#include <mutex>

namespace InMemoryDB {

class StorageEngine {
private:
    std::unordered_map<std::string, std::unique_ptr<Table>> tables_;
    mutable std::mutex mutex_;

public:
    StorageEngine() = default;
    ~StorageEngine() = default;

    // Table operations
    bool createTable(const std::string& name, const std::vector<Column>& columns);
    bool dropTable(const std::string& name);
    Table* getTable(const std::string& name);
    std::vector<std::string> getTableNames() const;

    // Transaction support
    void beginTransaction();
    void commit();
    void rollback();
};

}

#endif