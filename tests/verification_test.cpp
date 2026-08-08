#include "verification/Verification.h"

#include <cassert>
#include <fstream>
#include <string>

namespace {

void writeFile(const std::string& path, const std::string& contents) {
    std::ofstream out(path.c_str(), std::ios::binary);
    out << contents;
}

}  // namespace

int main() {
    const std::string clusterPath = "verify_cluster.ini";
    const std::string schemaPath = "verify_schema.ini";
    const std::string inputA = "verify_input_a.csv";
    const std::string inputB = "verify_input_b.csv";

    writeFile(clusterPath,
              "[cluster]\n"
              "node_count=2\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=" + inputA + "\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=" + inputB + "\n");

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

    writeFile(inputA, "1,alice\n2,bob\n");
    writeFile(inputB, "3,charlie\n");

    datastorage::Config config(clusterPath);
    datastorage::SchemaLoader schema(schemaPath);
    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader loader(config.cluster(), schema.schema(), partitioner);
    loader.load();

    datastorage::Verification verification(loader, partitioner);
    const datastorage::VerificationResult result = verification.verify();
    assert(result.passed);

    std::remove(clusterPath.c_str());
    std::remove(schemaPath.c_str());
    std::remove(inputA.c_str());
    std::remove(inputB.c_str());

    return 0;
}
