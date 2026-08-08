#ifndef DATASTORAGE_STATISTICS_H
#define DATASTORAGE_STATISTICS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace datastorage {

struct NodeStatistics {
    std::int32_t nodeId = 0;
    std::size_t recordsLoaded = 0;
    std::size_t recordsReceived = 0;
    std::size_t recordsTransferred = 0;
};

struct ClusterStatistics {
    std::size_t totalRecordsRead = 0;
    std::size_t validRecords = 0;
    std::size_t invalidRecords = 0;
    std::size_t duplicateRecords = 0;
    std::size_t recordsLoaded = 0;
    std::size_t recordsStored = 0;
    std::size_t recordsTransferred = 0;
    std::size_t recordsReceived = 0;
    std::vector<NodeStatistics> perNode;
};

class StatisticsCollector {
public:
    explicit StatisticsCollector(std::size_t nodeCount);

    void recordRead(std::int32_t nodeId, std::size_t count = 1u);
    void recordValid(std::size_t count = 1u);
    void recordInvalid(std::size_t count = 1u);
    void recordDuplicate(std::size_t count = 1u);
    void recordStored(std::int32_t nodeId, std::size_t count = 1u);
    void recordTransferred(std::int32_t nodeId, std::size_t count = 1u);
    void recordReceived(std::int32_t nodeId, std::size_t count = 1u);

    ClusterStatistics snapshot() const;

private:
    ClusterStatistics stats_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_STATISTICS_H
