#include "loader/Loader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "serialization/Serializer.h"

namespace datastorage {

namespace {

std::string trimCopy(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r')) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n')) {
        --end;
    }

    return value.substr(start, end - start);
}

}  // namespace

Loader::Loader(const ClusterConfig& clusterConfig, const Schema& schema, const IPartitioner& partitioner)
    : clusterConfig_(clusterConfig),
      schema_(schema),
      partitioner_(partitioner),
      parser_(schema_),
      nodeToIndex_(),
      stores_(),
      transports_(),
      stats_(),
      traceCount_(0u) {
    if (clusterConfig_.nodeCount <= 0) {
        throw std::runtime_error("Loader requires a positive node count");
    }
    if (schema_.fields.empty()) {
        throw std::runtime_error("Loader requires a non-empty schema");
    }
    initializeStores();
}

std::string Loader::trim(const std::string& value) {
    return trimCopy(value);
}

void Loader::initializeStores() {
    stores_.clear();
    transports_.clear();
    nodeToIndex_.clear();

    for (std::size_t i = 0; i < clusterConfig_.nodes.size(); ++i) {
        const std::int32_t nodeId = clusterConfig_.nodes[i].id;
        nodeToIndex_[nodeId] = i;
        stores_.push_back(std::unique_ptr<IKeyValueStore>(new InMemoryKeyValueStore()));
        transports_.push_back(MockSocketTransport(nodeId));
    }

    if (stores_.size() != static_cast<std::size_t>(clusterConfig_.nodeCount)) {
        throw std::runtime_error("Node store count does not match configured node_count");
    }
}

void Loader::processRecord(const std::string& line, std::int32_t nodeId) {
    if (trim(line).empty()) {
        return;
    }

    try {
        const Record record = parser_.parse(line);

        const NodeId owner = partitioner_.owner(record, clusterConfig_.nodeCount);
        const std::size_t ownerIndex = static_cast<std::size_t>(owner);
        const std::int32_t ownerNodeId = clusterConfig_.nodes[ownerIndex].id;
        const std::uint64_t hash = partitioner_.hashValue(record);

        ++stats_.recordsRead;
        ++stats_.validRecords;

        if (traceCount_ < 3u && ownerNodeId != nodeId) {
            std::cout << "| key=" << record.key
                      << " | current_node=" << nodeId
                      << " | destination_node=" << ownerNodeId
                      << " | hash=" << hash
                      << " | owner_index(hash % nodeCount)=" << ownerIndex
                      << " |" << '\n';
            ++traceCount_;
        }

        if (stores_[ownerIndex]->contains(record.key)) {
            ++stats_.duplicateRecords;
            return;
        }

        if (ownerNodeId == nodeId) {
            stores_[ownerIndex]->put(record.key, record);
            ++stats_.recordsStored;
            return;
        }

        std::string payload = Serializer::serialize(record);
        transports_[ownerIndex].connect(ownerNodeId);
        transports_[ownerIndex].send(payload);
        ++stats_.recordsTransferred;
    } catch (const std::exception&) {
        ++stats_.invalidRecords;
    }
}

void Loader::deliverQueuedRecords() {
    for (std::size_t ownerIndex = 0; ownerIndex < transports_.size(); ++ownerIndex) {
        while (transports_[ownerIndex].queuedMessages() > 0u) {
            const std::string payload = transports_[ownerIndex].recv();
            const Record record = Serializer::deserialize(payload);

            if (stores_[ownerIndex]->contains(record.key)) {
                ++stats_.duplicateRecords;
                continue;
            }

            stores_[ownerIndex]->put(record.key, record);
            ++stats_.recordsStored;
            ++stats_.recordsReceived;
        }
    }
}

std::string Loader::buildExpectedHeader() const {
    if (schema_.fields.empty()) {
        return "";
    }
    
    std::string header = schema_.fields[0].name;
    for (std::size_t i = 1; i < schema_.fields.size(); ++i) {
        header += "," + schema_.fields[i].name;
    }
    return header;
}

void Loader::load() {
    const std::string expectedHeader = buildExpectedHeader();
    
    for (std::size_t i = 0; i < clusterConfig_.nodes.size(); ++i) {
        const NodeConfig& node = clusterConfig_.nodes[i];
        std::ifstream input(node.inputFile.c_str());
        if (!input) {
            throw std::runtime_error("Failed to open input file: " + node.inputFile);
        }

        std::string line;
        bool firstLine = true;
        while (std::getline(input, line)) {
            // Check if first line matches schema field names
            if (firstLine) {
                firstLine = false;
                if (trim(line) == expectedHeader) {
                    continue; // Skip header line
                }
            }
            processRecord(line, node.id);
        }
    }

    deliverQueuedRecords();
}

const LoadStats& Loader::stats() const noexcept {
    return stats_;
}

const std::vector<std::unique_ptr<IKeyValueStore>>& Loader::stores() const noexcept {
    return stores_;
}

const std::vector<MockSocketTransport>& Loader::transports() const noexcept {
    return transports_;
}

}  // namespace datastorage
