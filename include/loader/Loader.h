#ifndef DATASTORAGE_LOADER_H
#define DATASTORAGE_LOADER_H

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

#include "core/Config.h"
#include "core/Schema.h"
#include "network/Transport.h"
#include "partition/Partitioner.h"
#include "record/Record.h"
#include "storage/Store.h"

namespace datastorage {

struct LoadStats {
    std::size_t recordsRead = 0;
    std::size_t validRecords = 0;
    std::size_t invalidRecords = 0;
    std::size_t duplicateRecords = 0;
    std::size_t recordsStored = 0;
    std::size_t recordsTransferred = 0;
    std::size_t recordsReceived = 0;
};

class Loader {
public:
    Loader(const ClusterConfig& clusterConfig, const Schema& schema, const IPartitioner& partitioner);

    void load();

    const LoadStats& stats() const noexcept;
    const std::vector<std::unique_ptr<IKeyValueStore>>& stores() const noexcept;
    const std::vector<MockSocketTransport>& transports() const noexcept;

private:
    static std::string trim(const std::string& value);
    void initializeStores();
    void processRecord(const std::string& line, std::int32_t nodeId);
    void deliverQueuedRecords();
    std::string buildExpectedHeader() const;

    ClusterConfig clusterConfig_;
    Schema schema_;
    const IPartitioner& partitioner_;
    RecordParser parser_;
    std::map<std::int32_t, std::size_t> nodeToIndex_;
    std::vector<std::unique_ptr<IKeyValueStore>> stores_;
    std::vector<MockSocketTransport> transports_;
    LoadStats stats_;
    std::size_t traceCount_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_LOADER_H
