#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "verification/Verification.h"

#include <cassert>
#include <fstream>
#include <string>

namespace {

void writeFile(const std::string& path, const std::string& contents) {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to create fixture: " + path);
    }
    out << contents;
}

}  // namespace

int main() {
    const std::string clusterPath = "e2e_cluster.ini";
    const std::string schemaPath = "e2e_schema.ini";
    const std::string input1Path = "e2e_node1.csv";
    const std::string input2Path = "e2e_node2.csv";
    const std::string input3Path = "e2e_node3.csv";

    writeFile(clusterPath,
              "[cluster]\n"
              "node_count=3\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=" + input1Path + "\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=" + input2Path + "\n\n"
              "[node.3]\n"
              "id=3\n"
              "input_file=" + input3Path + "\n");

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

    writeFile(input1Path, "1,alice\n4,diana\n");
    writeFile(input2Path, "2,bob\n5,erin\n");
    writeFile(input3Path, "3,charlie\n6,frank\n");

    datastorage::Config cluster(clusterPath);
    datastorage::SchemaLoader schema(schemaPath);
    datastorage::ModuloPartitioner partitioner;

    datastorage::Loader loader(cluster.cluster(), schema.schema(), partitioner);
    loader.load();

    assert(loader.stats().recordsRead == 6u);
    assert(loader.stats().validRecords == 6u);
    assert(loader.stats().recordsStored >= 1u);

    datastorage::Verification verifier(loader, partitioner);
    const datastorage::VerificationResult result = verifier.verify();
    assert(result.passed);

    std::remove(clusterPath.c_str());
    std::remove(schemaPath.c_str());
    std::remove(input1Path.c_str());
    std::remove(input2Path.c_str());
    std::remove(input3Path.c_str());

    return 0;
}
