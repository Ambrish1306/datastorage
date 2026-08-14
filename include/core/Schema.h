#ifndef DATASTORAGE_SCHEMA_H
#define DATASTORAGE_SCHEMA_H

#include <string>
#include <vector>

namespace datastorage {

enum class FieldType {
    String,
    Int32
};

struct FieldDefinition {
    std::string name;
    FieldType type = FieldType::String;
};

struct Schema {
    std::vector<FieldDefinition> fields;
    std::string keyField;
};
//
class SchemaLoader {
public:
    SchemaLoader();
    explicit SchemaLoader(const std::string& schemaPath);

    const Schema& schema() const noexcept;
    void loadFromFile(const std::string& schemaPath);

private:
    static std::string trim(const std::string& value);
    static FieldType parseFieldType(const std::string& rawType);

    Schema schema_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_SCHEMA_H
