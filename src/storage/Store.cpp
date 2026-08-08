#include "storage/Store.h"

#include <stdexcept>

namespace datastorage {

InMemoryKeyValueStore::InMemoryKeyValueStore() = default;

void InMemoryKeyValueStore::put(const std::string& key, const Record& record) {
    if (key.empty()) {
        throw std::runtime_error("Key cannot be empty when storing a record");
    }

    data_[key] = record;
}

bool InMemoryKeyValueStore::get(const std::string& key, Record* record) const {
    if (record == nullptr) {
        throw std::runtime_error("Record output pointer cannot be null");
    }

    const auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }

    *record = it->second;
    return true;
}

bool InMemoryKeyValueStore::contains(const std::string& key) const {
    return data_.find(key) != data_.end();
}

std::vector<std::string> InMemoryKeyValueStore::keys() const {
    std::vector<std::string> result;
    result.reserve(data_.size());
    for (const auto& entry : data_) {
        result.push_back(entry.first);
    }
    return result;
}

std::size_t InMemoryKeyValueStore::size() const {
    return data_.size();
}

void InMemoryKeyValueStore::clear() {
    data_.clear();
}

}  // namespace datastorage
