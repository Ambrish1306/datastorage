#include "record/Record.h"

#include <climits>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace datastorage {

namespace {

std::string trimCopy(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r')) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n')) {
        --end;
    }

    return value.substr(start, end - start);
}

}  // namespace

RecordParser::RecordParser(const Schema& schema)
    : schema_(schema) {
    if (schema_.fields.empty()) {
        throw std::runtime_error("RecordParser requires a non-empty schema");
    }
    if (schema_.keyField.empty()) {
        throw std::runtime_error("RecordParser requires a key field in the schema");
    }
}

std::string RecordParser::trim(const std::string& value) {
    return trimCopy(value);
}

std::vector<std::string> RecordParser::splitCsv(const std::string& text) {
    std::vector<std::string> tokens;
    tokens.reserve(8);

    std::string::size_type start = 0;
    while (true) {
        const std::string::size_type comma = text.find(',', start);
        if (comma == std::string::npos) {
            tokens.push_back(text.substr(start));
            break;
        }

        tokens.push_back(text.substr(start, comma - start));
        start = comma + 1;
    }

    return tokens;
}

FieldValue RecordParser::parseField(const std::string& rawValue, FieldType fieldType) const {
    const std::string value = trim(rawValue);
    FieldValue field;
    field.type = fieldType;

    if (fieldType == FieldType::String) {
        field.stringValue = value;
        return field;
    }

    if (fieldType == FieldType::Int32) {
        std::istringstream stream(value);
        long long parsedValue = 0;
        char trailing = 0;

        if (!(stream >> parsedValue) || (stream >> trailing)) {
            throw std::runtime_error("Invalid int32 value: " + value);
        }

        if (parsedValue < std::numeric_limits<std::int32_t>::min() ||
            parsedValue > std::numeric_limits<std::int32_t>::max()) {
            throw std::runtime_error("int32 value out of range: " + value);
        }

        field.int32Value = static_cast<std::int32_t>(parsedValue);
        return field;
    }

    throw std::runtime_error("Unsupported field type while parsing record");
}

Record RecordParser::parse(const std::string& rawRecord) const {
    if (rawRecord.empty()) {
        throw std::runtime_error("Cannot parse an empty record");
    }

    const std::vector<std::string> cells = splitCsv(rawRecord);
    if (cells.size() != schema_.fields.size()) {
        throw std::runtime_error("Record field count mismatch: expected " + std::to_string(schema_.fields.size()) +
                                 " but saw " + std::to_string(cells.size()));
    }

    Record record;
    record.fields.reserve(schema_.fields.size());

    for (std::size_t i = 0; i < schema_.fields.size(); ++i) {
        const FieldDefinition& fieldDefinition = schema_.fields[i];
        const FieldValue parsedValue = parseField(cells[i], fieldDefinition.type);
        record.fields.push_back(parsedValue);

        if (fieldDefinition.name == schema_.keyField) {
            if (fieldDefinition.type == FieldType::Int32) {
                record.key = std::to_string(parsedValue.int32Value);
            } else {
                record.key = parsedValue.stringValue;
            }
        }
    }

    if (record.key.empty()) {
        throw std::runtime_error("Key field '" + schema_.keyField + "' was not found in the record");
    }

    return record;
}

}  // namespace datastorage
