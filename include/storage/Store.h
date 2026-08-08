#ifndef DATASTORAGE_STORE_H
#define DATASTORAGE_STORE_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "record/Record.h"

namespace datastorage {

class IKeyValueStore {
public:
    virtual ~IKeyValueStore() = default;

    virtual void put(const std::string& key, const Record& record) = 0;
    virtual bool get(const std::string& key, Record* record) const = 0;
    virtual bool contains(const std::string& key) const = 0;
    virtual std::vector<std::string> keys() const = 0;
    virtual std::size_t size() const = 0;
    virtual void clear() = 0;
};

class InMemoryKeyValueStore : public IKeyValueStore {
public:
    InMemoryKeyValueStore();

    void put(const std::string& key, const Record& record) override;
    bool get(const std::string& key, Record* record) const override;
    bool contains(const std::string& key) const override;
    std::vector<std::string> keys() const override;
    std::size_t size() const override;
    void clear() override;

private:
    std::unordered_map<std::string, Record> data_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_STORE_H
