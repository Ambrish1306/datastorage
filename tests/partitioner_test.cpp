#include "partition/Partitioner.h"

#include <cassert>
#include <stdexcept>

int main() {
    datastorage::ModuloPartitioner partitioner;

    datastorage::Schema schema;
    schema.keyField = "id";
    schema.fields.push_back(datastorage::FieldDefinition{"id", datastorage::FieldType::Int32});
    schema.fields.push_back(datastorage::FieldDefinition{"name", datastorage::FieldType::String});

    datastorage::RecordParser parser(schema);
    datastorage::Record record = parser.parse("7,alice");

    const datastorage::NodeId owner = partitioner.owner(record, 3);
    assert(owner >= 0);
    assert(owner < 3);

    bool invalid = false;
    try {
        partitioner.owner(datastorage::Record(), 3);
    } catch (const std::runtime_error&) {
        invalid = true;
    }
    assert(invalid);

    return 0;
}
