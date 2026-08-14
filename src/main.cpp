#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "partition/Partitioner.h"
#include "verification/Verification.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct AppOptions {
    std::string clusterPath = "config/cluster.ini";
    std::string schemaPath = "config/schema.ini";
};

AppOptions parseOptions(int argc, char** argv) {
    AppOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--cluster" || arg == "-c") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for --cluster");
            }
            options.clusterPath = argv[++i];
            continue;
        }

        if (arg == "--schema" || arg == "-s") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for --schema");
            }
            options.schemaPath = argv[++i];
            continue;
        }

        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: datastorage [--cluster path] [--schema path]\n";
            std::exit(0);
        }

        throw std::runtime_error("Unknown argument: " + arg);
    }

    return options;
}

void printStatistics(const datastorage::Loader& loader, const datastorage::ClusterConfig& clusterConfig) {
    const datastorage::LoadStats& stats = loader.stats();

    std::cout << "\nStatistics:\n";
    std::cout << "  Total records read: " << stats.recordsRead << '\n';
    std::cout << "  Valid records: " << stats.validRecords << '\n';
    std::cout << "  Invalid records: " << stats.invalidRecords << '\n';
    std::cout << "  Duplicate records: " << stats.duplicateRecords << '\n';
    std::cout << "  Records stored(unique records kept in storage): " << stats.recordsStored << '\n';
    std::cout << "  Records transferred: " << stats.recordsTransferred << '\n';
    std::cout << "  Records received: " << stats.recordsReceived << '\n';
    std::cout << "  Records loaded per node:\n";

    for (std::size_t i = 0; i < clusterConfig.nodes.size(); ++i) {
        const std::int32_t nodeId = clusterConfig.nodes[i].id;
        const std::size_t loaded = loader.stores()[i]->size();
        std::cout << "    Node " << nodeId << ": " << loaded << '\n';
    }
}

void printVerification(const datastorage::VerificationResult& result) {
    std::cout << "\nVerification result: " << (result.passed ? "PASS" : "FAIL") << '\n';
    std::cout << "  Records checked: " << result.recordsChecked << '\n';
    std::cout << "  Incorrect owner: " << result.incorrectOwner << '\n';
    std::cout << "  Missing records: " << result.missingRecords << '\n';
    std::cout << "  Duplicate owners: " << result.duplicateOwners << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const AppOptions options = parseOptions(argc, argv);

        datastorage::Config clusterConfig(options.clusterPath);
        datastorage::SchemaLoader schemaLoader(options.schemaPath);
        datastorage::ModuloPartitioner partitioner;
        datastorage::Loader loader(clusterConfig.cluster(), schemaLoader.schema(), partitioner);

        loader.load();

        datastorage::Verification verifier(loader, partitioner);
        const datastorage::VerificationResult verificationResult = verifier.verify();

        std::cout << "Cluster node count: " << clusterConfig.cluster().nodeCount << '\n';
        std::cout << "Schema key field: " << schemaLoader.schema().keyField << '\n';
        printStatistics(loader, clusterConfig.cluster());
        printVerification(verificationResult);

        return verificationResult.passed ? 0 : 1;
    } catch (const std::exception& ex) {
        std::cerr << "Initialization failed: " << ex.what() << std::endl;
        return 1;
    }
}
