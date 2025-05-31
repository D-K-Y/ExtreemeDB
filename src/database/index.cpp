#include "types.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>

namespace InMemoryDB {

class Index {
public:
    virtual ~Index() = default;
    virtual void insert(const Value& key, int row_id) = 0;
    virtual void remove(const Value& key, int row_id) = 0;
    virtual std::vector<int> find(const Value& key) = 0;
    virtual std::vector<int> findRange(const Value& start, const Value& end) = 0;
};

// Hash-based index for equality searches
class HashIndex : public Index {
private:
    std::unordered_map<std::string, std::vector<int>> index_;
    
    std::string valueToString(const Value& value) {
        return std::visit([](const auto& v) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
                return v;
            } else {
                return std::to_string(v);
            }
        }, value);
    }
    
public:
    void insert(const Value& key, int row_id) override {
        std::string key_str = valueToString(key);
        index_[key_str].push_back(row_id);
    }
    
    void remove(const Value& key, int row_id) override {
        std::string key_str = valueToString(key);
        auto& row_ids = index_[key_str];
        row_ids.erase(std::remove(row_ids.begin(), row_ids.end(), row_id), row_ids.end());
        
        if (row_ids.empty()) {
            index_.erase(key_str);
        }
    }
    
    std::vector<int> find(const Value& key) override {
        std::string key_str = valueToString(key);
        auto it = index_.find(key_str);
        if (it != index_.end()) {
            return it->second;
        }
        return {};
    }
    
    std::vector<int> findRange(const Value& start, const Value& end) override {
        // Hash index doesn't support range queries efficiently
        return {};
    }
};

// Tree-based index for range searches
class TreeIndex : public Index {
private:
    std::map<std::string, std::vector<int>> index_;
    
    std::string valueToString(const Value& value) {
        return std::visit([](const auto& v) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
                return v;
            } else {
                return std::to_string(v);
            }
        }, value);
    }
    
public:
    void insert(const Value& key, int row_id) override {
        std::string key_str = valueToString(key);
        index_[key_str].push_back(row_id);
    }
    
    void remove(const Value& key, int row_id) override {
        std::string key_str = valueToString(key);
        auto& row_ids = index_[key_str];
        row_ids.erase(std::remove(row_ids.begin(), row_ids.end(), row_id), row_ids.end());
        
        if (row_ids.empty()) {
            index_.erase(key_str);
        }
    }
    
    std::vector<int> find(const Value& key) override {
        std::string key_str = valueToString(key);
        auto it = index_.find(key_str);
        if (it != index_.end()) {
            return it->second;
        }
        return {};
    }
    
    std::vector<int> findRange(const Value& start, const Value& end) override {
        std::vector<int> result;
        std::string start_str = valueToString(start);
        std::string end_str = valueToString(end);
        
        auto start_it = index_.lower_bound(start_str);
        auto end_it = index_.upper_bound(end_str);
        
        for (auto it = start_it; it != end_it; ++it) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
        
        return result;
    }
};

}