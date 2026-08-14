#include "loader/Loader.h"

#include <cassert>
#include <fstream>
#include <stdexcept>
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
    const std::string clusterPath = "loader_cluster_test.ini";
    const std::string schemaPath = "loader_schema_test.ini";
    const std::string node1Path = "loader_input_1.csv";
    const std::string node2Path = "loader_input_2.csv";

    writeFile(clusterPath,
              "[cluster]\n"
              "node_count=2\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=" + node1Path + "\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=" + node2Path + "\n");

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

    writeFile(node1Path, "1,alice\n2,bob\n");
    writeFile(node2Path, "3,charlie\n");

    datastorage::Config cluster(clusterPath);
    datastorage::SchemaLoader schema(schemaPath);

    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader loader(cluster.cluster(), schema.schema(), partitioner);
    loader.load();

    assert(loader.stats().recordsRead == 3u);
    assert(loader.stats().recordsStored == 3u);
    assert(loader.stats().recordsTransferred == 0u || loader.stats().recordsTransferred <= 3u);

    // Remote records must be delivered through the destination node's transport queue and then received.
    writeFile("remote_transfer_cluster.ini",
              "[cluster]\n"
              "node_count=2\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=remote_transfer_node1.csv\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=remote_transfer_node2.csv\n");
    writeFile("remote_transfer_schema.ini",
              "[schema]\n"
              "key_field=id\n"
              "field_count=2\n\n"
              "[field.1]\n"
              "name=id\n"
              "type=int32\n\n"
              "[field.2]\n"
              "name=name\n"
              "type=string\n");
    writeFile("remote_transfer_node1.csv", "2,bob\n");
    writeFile("remote_transfer_node2.csv", "1,alice\n");

    datastorage::Config remoteCluster("remote_transfer_cluster.ini");
    datastorage::SchemaLoader remoteSchema("remote_transfer_schema.ini");
    datastorage::Loader remoteLoader(remoteCluster.cluster(), remoteSchema.schema(), partitioner);
    remoteLoader.load();
    assert(remoteLoader.stats().recordsTransferred >= 1u);
    assert(remoteLoader.stats().recordsReceived >= 1u);

    std::remove(clusterPath.c_str());
    std::remove(schemaPath.c_str());
    std::remove(node1Path.c_str());
    std::remove(node2Path.c_str());
    std::remove("remote_transfer_cluster.ini");
    std::remove("remote_transfer_schema.ini");
    std::remove("remote_transfer_node1.csv");
    std::remove("remote_transfer_node2.csv");

    return 0;
}
