#include "core/Config.h"
#include "loader/Loader.h"

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

void writeFile(const std::string& path, const std::string& contents) {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to write fixture: " + path);
    }
    out << contents;
}

}  // namespace

int main() {
    const std::string badConfigPath = "bad_cluster.ini";
    writeFile(badConfigPath,
              "[cluster]\n"
              "node_count=0\n");

    bool invalidConfig = false;
    try {
        datastorage::Config config(badConfigPath);
        (void)config;
    } catch (const std::runtime_error&) {
        invalidConfig = true;
    }
    assert(invalidConfig);
    std::remove(badConfigPath.c_str());

    const std::string clusterPath = "unit_loader_cluster.ini";
    const std::string schemaPath = "unit_loader_schema.ini";
    const std::string inputPath = "unit_loader_input.csv";

    writeFile(clusterPath,
              "[cluster]\n"
              "node_count=1\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=" + inputPath + "\n");

    writeFile(schemaPath,
              "[schema]\n"
              "key_field=id\n"
              "field_count=2\n\n"
              "[field.1]\n"
              "name=id\n"
              "type=int32\n\n"
              "[field.2]\n"
              "name=name\n"
              "type=string\n");

    writeFile(inputPath, "1,alice\n1,alice\n2,bob\n");

    datastorage::Config cluster(clusterPath);
    datastorage::SchemaLoader schema(schemaPath);
    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader loader(cluster.cluster(), schema.schema(), partitioner);
    loader.load();

    assert(loader.stats().recordsRead == 3u);
    assert(loader.stats().duplicateRecords == 1u);
    assert(loader.stats().validRecords == 3u);
    assert(loader.stats().invalidRecords == 0u);
    assert(loader.stats().recordsStored == 2u);

    std::remove(clusterPath.c_str());
    std::remove(schemaPath.c_str());
    std::remove(inputPath.c_str());

    return 0;
}
