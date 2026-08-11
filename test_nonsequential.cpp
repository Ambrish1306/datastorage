#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "partition/Partitioner.h"
#include <fstream>
#include <iostream>

// Test with non-sequential node IDs: 10, 20, 30 instead of 1, 2, 3
int main() {
    // Create test cluster config with non-sequential IDs
    std::ofstream clusterFile("test_nonseq_cluster.ini");
    clusterFile << "[cluster]\n"
                << "node_count=3\n\n"
                << "[node.1]\n"
                << "id=10\n"          // Non-sequential!
                << "input_file=test_node10.csv\n\n"
                << "[node.2]\n"
                << "id=20\n"          // Non-sequential!
                << "input_file=test_node20.csv\n\n"
                << "[node.3]\n"
                << "id=30\n"          // Non-sequential!
                << "input_file=test_node30.csv\n";
    clusterFile.close();

    // Create test schema
    std::ofstream schemaFile("test_schema.ini");
    schemaFile << "[schema]\n"
               << "field_count=2\n"
               << "key_field=id\n\n"
               << "[field.1]\n"
               << "name=id\n"
               << "type=int32\n\n"
               << "[field.2]\n"
               << "name=value\n"
               << "type=string\n";
    schemaFile.close();

    // Create test data files
    std::ofstream node10("test_node10.csv");
    node10 << "1,data1\n2,data2\n3,data3\n";
    node10.close();

    std::ofstream node20("test_node20.csv");
    node20 << "4,data4\n5,data5\n6,data6\n";
    node20.close();

    std::ofstream node30("test_node30.csv");
    node30 << "7,data7\n8,data8\n9,data9\n";
    node30.close();

    // Load and run
    datastorage::Config config("test_nonseq_cluster.ini");
    datastorage::SchemaLoader schema("test_schema.ini");
    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader loader(config.cluster(), schema.schema(), partitioner);

    loader.load();

    const datastorage::LoadStats& stats = loader.stats();
    
    std::cout << "\n=== BUG DEMONSTRATION: Non-Sequential Node IDs ===\n";
    std::cout << "Node IDs in config: 10, 20, 30\n";
    std::cout << "Partitioner returns: 0, 1, 2 (modulo result)\n\n";
    std::cout << "Statistics:\n";
    std::cout << "  Records read: " << stats.recordsRead << "\n";
    std::cout << "  Records transferred: " << stats.recordsTransferred << "\n\n";
    
    std::cout << "❌ BUG: The code compares:\n";
    std::cout << "   owner (0-2) != nodeId (10, 20, 30)\n";
    std::cout << "   This comparison is ALWAYS true!\n";
    std::cout << "   Result: ALL records marked as 'transferred' even if local!\n\n";
    std::cout << "Expected transferred: ~6 records (66% go to other nodes)\n";
    std::cout << "Actual transferred: " << stats.recordsTransferred << " records (likely ALL 9)\n";

    return 0;
}
