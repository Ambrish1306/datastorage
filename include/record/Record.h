#ifndef DATASTORAGE_RECORD_H
#define DATASTORAGE_RECORD_H

#include <cstdint>
#include <string>
#include <vector>

#include "core/Schema.h"

namespace datastorage {

struct FieldValue {
    FieldType type = FieldType::String;
    std::string stringValue;
    std::int32_t int32Value = 0;
};

struct Record {
    std::string key;
    std::vector<FieldValue> fields;
};

class RecordParser {
public:
    explicit RecordParser(const Schema& schema);

    Record parse(const std::string& rawRecord) const;

private:
    Schema schema_;
    static std::string trim(const std::string& value);
    static std::vector<std::string> splitCsv(const std::string& text);
    FieldValue parseField(const std::string& rawValue, FieldType fieldType) const;
};

}  // namespace datastorage

#endif  // DATASTORAGE_RECORD_H
