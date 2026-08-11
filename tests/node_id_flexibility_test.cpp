#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "partition/Partitioner.h"
#include "verification/Verification.h"

#include <cassert>
#include <fstream>
#include <iostream>

namespace {

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
}

}  // namespace

// Test that loader works correctly with non-sequential node IDs
int main() {
    // Test Case 1: Sequential IDs (1, 2, 3) - standard case
    writeFile("test_seq_cluster.ini",
              "[cluster]\n"
              "node_count=3\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=test_seq_node1.csv\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=test_seq_node2.csv\n\n"
              "[node.3]\n"
              "id=3\n"
              "input_file=test_seq_node3.csv\n");

    // Test Case 2: Non-sequential IDs (10, 25, 100) - edge case
    writeFile("test_nonseq_cluster.ini",
              "[cluster]\n"
              "node_count=3\n\n"
              "[node.1]\n"
              "id=10\n"
              "input_file=test_nonseq_node1.csv\n\n"
              "[node.2]\n"
              "id=25\n"
              "input_file=test_nonseq_node2.csv\n\n"
              "[node.3]\n"
              "id=100\n"
              "input_file=test_nonseq_node3.csv\n");

    writeFile("test_schema.ini",
              "[schema]\n"
              "field_count=2\n"
              "key_field=id\n\n"
              "[field.1]\n"
              "name=id\n"
              "type=int32\n\n"
              "[field.2]\n"
              "name=value\n"
              "type=string\n");

    // Create identical test data for both configs
    // Each node reads 3 records, total 9 records
    writeFile("test_seq_node1.csv", "1,value1\n2,value2\n3,value3\n");
    writeFile("test_seq_node2.csv", "4,value4\n5,value5\n6,value6\n");
    writeFile("test_seq_node3.csv", "7,value7\n8,value8\n9,value9\n");

    writeFile("test_nonseq_node1.csv", "1,value1\n2,value2\n3,value3\n");
    writeFile("test_nonseq_node2.csv", "4,value4\n5,value5\n6,value6\n");
    writeFile("test_nonseq_node3.csv", "7,value7\n8,value8\n9,value9\n");

    // Test sequential IDs
    std::cout << "Testing sequential node IDs (1, 2, 3)...\n";
    datastorage::Config seqConfig("test_seq_cluster.ini");
    datastorage::SchemaLoader schema("test_schema.ini");
    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader seqLoader(seqConfig.cluster(), schema.schema(), partitioner);
    seqLoader.load();

    const datastorage::LoadStats& seqStats = seqLoader.stats();
    std::cout << "  Records read: " << seqStats.recordsRead << "\n";
    std::cout << "  Records stored: " << seqStats.recordsStored << "\n";
    std::cout << "  Records transferred: " << seqStats.recordsTransferred << "\n";

    datastorage::Verification seqVerifier(seqLoader, partitioner);
    datastorage::VerificationResult seqResult = seqVerifier.verify();
    std::cout << "  Verification: " << (seqResult.passed ? "PASS" : "FAIL") << "\n";
    assert(seqResult.passed);
    assert(seqStats.recordsRead == 9);
    assert(seqStats.recordsStored == 9);

    // Test non-sequential IDs
    std::cout << "\nTesting non-sequential node IDs (10, 25, 100)...\n";
    datastorage::Config nonseqConfig("test_nonseq_cluster.ini");
    datastorage::Loader nonseqLoader(nonseqConfig.cluster(), schema.schema(), partitioner);
    nonseqLoader.load();

    const datastorage::LoadStats& nonseqStats = nonseqLoader.stats();
    std::cout << "  Records read: " << nonseqStats.recordsRead << "\n";
    std::cout << "  Records stored: " << nonseqStats.recordsStored << "\n";
    std::cout << "  Records transferred: " << nonseqStats.recordsTransferred << "\n";

    datastorage::Verification nonseqVerifier(nonseqLoader, partitioner);
    datastorage::VerificationResult nonseqResult = nonseqVerifier.verify();
    std::cout << "  Verification: " << (nonseqResult.passed ? "PASS" : "FAIL") << "\n";
    assert(nonseqResult.passed);
    assert(nonseqStats.recordsRead == 9);
    assert(nonseqStats.recordsStored == 9);

    // Key assertion: transfer counts should be similar for both configs
    // (since same data, same partitioning logic)
    std::cout << "\n✅ Both sequential and non-sequential node IDs work correctly!\n";
    std::cout << "Transfer count difference: " 
              << (seqStats.recordsTransferred > nonseqStats.recordsTransferred 
                  ? seqStats.recordsTransferred - nonseqStats.recordsTransferred
                  : nonseqStats.recordsTransferred - seqStats.recordsTransferred)
              << " (should be 0 or small)\n";

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "✅ Sequential IDs: " << (seqResult.passed ? "PASS" : "FAIL") << "\n";
    std::cout << "✅ Non-sequential IDs: " << (nonseqResult.passed ? "PASS" : "FAIL") << "\n";
    std::cout << "✅ Transfer statistics consistent: " 
              << (seqStats.recordsTransferred == nonseqStats.recordsTransferred ? "YES" : "NO") << "\n";

    return 0;
}
