#include "core/Config.h"
#include "core/Schema.h"

#include <cassert>
#include <fstream>
#include <stdexcept>

namespace {

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
}

}  // namespace

int main() {
    writeFile("test_cluster_valid.ini",
              "[cluster]\n"
              "node_count=3\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=data/node1/input.csv\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=data/node2/input.csv\n\n"
              "[node.3]\n"
              "id=3\n"
              "input_file=data/node3/input.csv\n");

    writeFile("test_schema_valid.ini",
              "[schema]\n"
              "key_field=id\n\n"
              "[field.1]\n"
              "name=id\n"
              "type=int32\n\n"
              "[field.2]\n"
              "name=name\n"
              "type=string\n\n"
              "[field.3]\n"
              "name=country\n"
              "type=string\n");

    datastorage::Config config("test_cluster_valid.ini");
    datastorage::SchemaLoader schema("test_schema_valid.ini");

    assert(config.cluster().nodeCount == 3);
    assert(config.cluster().nodes.size() == 3u);
    assert(schema.schema().keyField == "id");
    assert(schema.schema().fields.size() == 3u);
    assert(schema.schema().fields[0].type == datastorage::FieldType::Int32);

    writeFile("test_cluster_invalid.ini",
              "[cluster]\n"
              "node_count=0\n");

    bool invalidCluster = false;
    try {
        datastorage::Config invalid("test_cluster_invalid.ini");
        (void)invalid;
    } catch (const std::runtime_error&) {
        invalidCluster = true;
    }
    assert(invalidCluster);

    writeFile("test_schema_invalid.ini",
              "[schema]\n"
              "key_field=id\n\n"
              "[field.1]\n"
              "name=id\n"
              "type=float\n");

    bool invalidSchema = false;
    try {
        datastorage::SchemaLoader badSchema("test_schema_invalid.ini");
        (void)badSchema;
    } catch (const std::runtime_error&) {
        invalidSchema = true;
    }
    assert(invalidSchema);

    return 0;
}
