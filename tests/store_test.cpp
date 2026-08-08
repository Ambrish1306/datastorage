#include "storage/Store.h"

#include <cassert>
#include <stdexcept>

int main() {
    datastorage::Schema schema;
    schema.keyField = "id";
    schema.fields.push_back(datastorage::FieldDefinition{"id", datastorage::FieldType::Int32});
    schema.fields.push_back(datastorage::FieldDefinition{"name", datastorage::FieldType::String});

    datastorage::RecordParser parser(schema);
    datastorage::Record record = parser.parse("42,alice");

    datastorage::InMemoryKeyValueStore store;
    store.put(record.key, record);

    assert(store.contains(record.key));
    assert(store.size() == 1u);

    datastorage::Record loaded;
    assert(store.get(record.key, &loaded));
    assert(loaded.key == record.key);
    assert(loaded.fields[0].int32Value == 42);

    store.clear();
    assert(store.size() == 0u);

    bool invalid = false;
    try {
        store.get("missing", nullptr);
    } catch (const std::runtime_error&) {
        invalid = true;
    }
    assert(invalid);

    return 0;
}
