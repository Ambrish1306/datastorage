#include "record/Record.h"

#include <cassert>
#include <stdexcept>

int main() {
    datastorage::Schema schema;
    schema.keyField = "id";
    schema.fields.push_back(datastorage::FieldDefinition{"id", datastorage::FieldType::Int32});
    schema.fields.push_back(datastorage::FieldDefinition{"name", datastorage::FieldType::String});
    schema.fields.push_back(datastorage::FieldDefinition{"country", datastorage::FieldType::String});

    datastorage::RecordParser parser(schema);
    datastorage::Record record = parser.parse("42,alice,USA");

    assert(record.key == "42");
    assert(record.fields.size() == 3u);
    assert(record.fields[0].type == datastorage::FieldType::Int32);
    assert(record.fields[0].int32Value == 42);
    assert(record.fields[1].stringValue == "alice");

    bool invalid = false;
    try {
        parser.parse("not-an-int,alice,USA");
    } catch (const std::runtime_error&) {
        invalid = true;
    }
    assert(invalid);

    return 0;
}
