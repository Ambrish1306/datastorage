#include "core/Schema.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

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

SchemaLoader::SchemaLoader() = default;

SchemaLoader::SchemaLoader(const std::string& schemaPath) {
    loadFromFile(schemaPath);
}

const Schema& SchemaLoader::schema() const noexcept {
    return schema_;
}

std::string SchemaLoader::trim(const std::string& value) {
    return trimCopy(value);
}

FieldType SchemaLoader::parseFieldType(const std::string& rawType) {
    const std::string type = trim(rawType);
    if (type == "string") {
        return FieldType::String;
    }
    if (type == "int32") {
        return FieldType::Int32;
    }
    throw std::runtime_error("Unsupported schema field type: " + type);
}

void SchemaLoader::loadFromFile(const std::string& schemaPath) {
    std::ifstream input(schemaPath.c_str());
    if (!input) {
        throw std::runtime_error("Failed to open schema config: " + schemaPath);
    }

    Schema schema;
    std::string line;
    std::string currentSection;
    std::vector<FieldDefinition> fields;
    bool sawSchemaSection = false;

    while (std::getline(input, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trim(trimmed.substr(1, trimmed.size() - 2));
            if (currentSection == "schema") {
                sawSchemaSection = true;
            }
            continue;
        }

        const std::size_t equalsPos = trimmed.find('=');
        if (equalsPos == std::string::npos) {
            throw std::runtime_error("Malformed line in schema config: " + line);
        }

        const std::string key = trim(trimmed.substr(0, equalsPos));
        const std::string value = trim(trimmed.substr(equalsPos + 1));

        if (currentSection == "schema") {
            if (key == "key_field") {
                schema.keyField = value;
            } else if (key == "field_count") {
                const int fieldCount = std::stoi(value);
                if (fieldCount <= 0) {
                    throw std::runtime_error("field_count must be positive");
                }
            } else {
                throw std::runtime_error("Unknown schema option: " + key);
            }
        } else if (currentSection.find("field.") == 0) {
            const std::string indexText = currentSection.substr(6);
            if (indexText.empty()) {
                throw std::runtime_error("Invalid field section: " + currentSection);
            }

            std::size_t fieldIndex = 0;
            try {
                fieldIndex = static_cast<std::size_t>(std::stoi(indexText));
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid field index: " + indexText);
            }

            if (fieldIndex == 0) {
                throw std::runtime_error("Field indices must start at 1");
            }

            while (fields.size() < fieldIndex) {
                fields.push_back(FieldDefinition());
            }

            if (key == "name") {
                if (value.empty()) {
                    throw std::runtime_error("Field name cannot be empty at index " + std::to_string(fieldIndex));
                }
                fields[fieldIndex - 1].name = value;
            } else if (key == "type") {
                fields[fieldIndex - 1].type = parseFieldType(value);
            } else {
                throw std::runtime_error("Unknown field option: " + key);
            }
        }
    }

    if (!sawSchemaSection) {
        throw std::runtime_error("Schema configuration is missing the [schema] section");
    }

    for (std::size_t i = 0; i < fields.size(); ++i) {
        const FieldDefinition& field = fields[i];
        if (field.name.empty()) {
            throw std::runtime_error("Field at index " + std::to_string(i + 1) + " is missing a name");
        }
        if (field.name == schema.keyField) {
            continue;
        }
    }

    if (schema.keyField.empty()) {
        throw std::runtime_error("Schema must define a key_field");
    }

    const bool keyFound = std::find_if(fields.begin(), fields.end(), [&schema](const FieldDefinition& field) {
        return field.name == schema.keyField;
    }) != fields.end();

    if (!keyFound) {
        throw std::runtime_error("key_field references an undefined field: " + schema.keyField);
    }

    if (fields.empty()) {
        throw std::runtime_error("Schema must define at least one field");
    }

    for (std::size_t i = 0; i < fields.size(); ++i) {
        for (std::size_t j = i + 1; j < fields.size(); ++j) {
            if (fields[i].name == fields[j].name) {
                throw std::runtime_error("Duplicate field name in schema: " + fields[i].name);
            }
        }
    }

    schema.fields = fields;
    schema_ = schema;
}

}  // namespace datastorage
