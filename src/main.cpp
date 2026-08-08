#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "partition/Partitioner.h"

#include <iostream>

namespace {

void printStatistics(const datastorage::Loader& loader, const datastorage::ClusterConfig& clusterConfig) {
    const datastorage::LoadStats& stats = loader.stats();

    std::cout << "\nStatistics:\n";
    std::cout << "  Total records read: " << stats.recordsRead << '\n';
    std::cout << "  Valid records: " << stats.validRecords << '\n';
    std::cout << "  Invalid records: " << stats.invalidRecords << '\n';
    std::cout << "  Duplicate records: " << stats.duplicateRecords << '\n';
    std::cout << "  Records stored: " << stats.recordsStored << '\n';
    std::cout << "  Records transferred: " << stats.recordsTransferred << '\n';
    std::cout << "  Records received: " << stats.recordsReceived << '\n';
    std::cout << "  Records loaded per node:\n";

    for (std::size_t i = 0; i < clusterConfig.nodes.size(); ++i) {
        const std::int32_t nodeId = clusterConfig.nodes[i].id;
        const std::size_t loaded = loader.stores()[i]->size();
        std::cout << "    Node " << nodeId << ": " << loaded << '\n';
    }
}

}  // namespace

int main() {
    try {
        datastorage::Config clusterConfig("config/cluster.ini");
        datastorage::SchemaLoader schemaLoader("config/schema.ini");
        datastorage::ModuloPartitioner partitioner;
        datastorage::Loader loader(clusterConfig.cluster(), schemaLoader.schema(), partitioner);

        loader.load();

        std::cout << "Cluster node count: " << clusterConfig.cluster().nodeCount << '\n';
        std::cout << "Schema key field: " << schemaLoader.schema().keyField << '\n';
        printStatistics(loader, clusterConfig.cluster());
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Initialization failed: " << ex.what() << std::endl;
        return 1;
    }
}
