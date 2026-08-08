#include "serialization/Serializer.h"

#include <cassert>
#include <cstdint>
#include <string>

int main() {
    datastorage::Record record;
    record.key = "42";

    datastorage::FieldValue id;
    id.type = datastorage::FieldType::Int32;
    id.int32Value = 0x01020304;

    datastorage::FieldValue name;
    name.type = datastorage::FieldType::String;
    name.stringValue = "alice";

    record.fields.push_back(id);
    record.fields.push_back(name);

    const std::string encoded = datastorage::Serializer::serialize(record);
    const datastorage::Record decoded = datastorage::Serializer::deserialize(encoded);

    assert(decoded.key == record.key);
    assert(decoded.fields.size() == record.fields.size());
    assert(decoded.fields[0].type == datastorage::FieldType::Int32);
    assert(decoded.fields[0].int32Value == 0x01020304);
    assert(decoded.fields[1].type == datastorage::FieldType::String);
    assert(decoded.fields[1].stringValue == "alice");

    const std::size_t valueOffset = 4u + record.key.size() + 4u + 4u + 4u;
    std::string expectedBytes;
    expectedBytes.resize(4u);
    std::uint32_t raw = static_cast<std::uint32_t>(0x01020304);
    for (std::size_t i = 0; i < 4u; ++i) {
        expectedBytes[i] = static_cast<char>((raw >> (8 * (3u - i))) & 0xFFu);
    }
    assert(encoded.substr(valueOffset, 4u) == expectedBytes);

    return 0;
}
