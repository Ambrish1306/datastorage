#include "core/Config.h"
#include "core/Schema.h"
#include "loader/Loader.h"
#include "verification/Verification.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
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

std::string buildRecordLine(std::int32_t id, const std::string& name) {
    std::ostringstream stream;
    stream << id << ',' << name;
    return stream.str();
}

}  // namespace

int main() {
    const std::size_t targetBytes = 100u * 1024u * 1024u;
    const std::string clusterPath = "benchmark_cluster.ini";
    const std::string schemaPath = "benchmark_schema.ini";
    const std::string input1Path = "benchmark_input_1.csv";
    const std::string input2Path = "benchmark_input_2.csv";

    writeFile(clusterPath,
              "[cluster]\n"
              "node_count=2\n\n"
              "[node.1]\n"
              "id=1\n"
              "input_file=" + input1Path + "\n\n"
              "[node.2]\n"
              "id=2\n"
              "input_file=" + input2Path + "\n");

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

    std::ofstream first(input1Path.c_str(), std::ios::binary);
    std::ofstream second(input2Path.c_str(), std::ios::binary);
    if (!first || !second) {
        throw std::runtime_error("Failed to open benchmark input files");
    }

    std::size_t bytesWritten = 0;
    std::int32_t id = 1;
    while (bytesWritten < targetBytes) {
        const std::string row = buildRecordLine(id, "benchmark_name") + "\n";
        first << row;
        second << buildRecordLine(id + 1000000, "benchmark_name") << '\n';
        bytesWritten += row.size() * 2u;
        ++id;
    }

    first.close();
    second.close();

    const auto start = std::chrono::steady_clock::now();

    datastorage::Config cluster(clusterPath);
    datastorage::SchemaLoader schema(schemaPath);
    datastorage::ModuloPartitioner partitioner;
    datastorage::Loader loader(cluster.cluster(), schema.schema(), partitioner);
    loader.load();

    datastorage::Verification verifier(loader, partitioner);
    const datastorage::VerificationResult result = verifier.verify();

    const auto end = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double recordsPerSecond = elapsedMs == 0 ? 0.0 : (static_cast<double>(loader.stats().recordsRead) * 1000.0 / static_cast<double>(elapsedMs));

    std::cout << "Benchmark data size target: " << targetBytes << " bytes\n";
    std::cout << "Records loaded: " << loader.stats().recordsRead << '\n';
    std::cout << "Records stored: " << loader.stats().recordsStored << '\n';
    std::cout << "Verification passed: " << (result.passed ? "yes" : "no") << '\n';
    std::cout << "Elapsed ms: " << elapsedMs << '\n';
    std::cout << "Records/sec: " << recordsPerSecond << '\n';

    assert(result.passed);
    assert(loader.stats().recordsRead > 0u);

    std::remove(clusterPath.c_str());
    std::remove(schemaPath.c_str());
    std::remove(input1Path.c_str());
    std::remove(input2Path.c_str());

    return 0;
}
