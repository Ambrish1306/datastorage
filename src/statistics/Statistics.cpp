#include "statistics/Statistics.h"

namespace datastorage {

StatisticsCollector::StatisticsCollector(std::size_t nodeCount) {
    stats_.perNode.resize(nodeCount);
    for (std::size_t i = 0; i < nodeCount; ++i) {
        stats_.perNode[i].nodeId = static_cast<std::int32_t>(i + 1);
    }
}

void StatisticsCollector::recordRead(std::int32_t nodeId, std::size_t count) {
    (void)nodeId;
    stats_.totalRecordsRead += count;
}

void StatisticsCollector::recordValid(std::size_t count) {
    stats_.validRecords += count;
}

void StatisticsCollector::recordInvalid(std::size_t count) {
    stats_.invalidRecords += count;
}

void StatisticsCollector::recordDuplicate(std::size_t count) {
    stats_.duplicateRecords += count;
}

void StatisticsCollector::recordStored(std::int32_t nodeId, std::size_t count) {
    stats_.recordsStored += count;
    stats_.recordsLoaded += count;
    if (static_cast<std::size_t>(nodeId) <= stats_.perNode.size()) {
        stats_.perNode[static_cast<std::size_t>(nodeId) - 1].recordsLoaded += count;
    }
}

void StatisticsCollector::recordTransferred(std::int32_t nodeId, std::size_t count) {
    stats_.recordsTransferred += count;
    if (static_cast<std::size_t>(nodeId) <= stats_.perNode.size()) {
        stats_.perNode[static_cast<std::size_t>(nodeId) - 1].recordsTransferred += count;
    }
}

void StatisticsCollector::recordReceived(std::int32_t nodeId, std::size_t count) {
    stats_.recordsReceived += count;
    stats_.recordsLoaded += count;
    if (static_cast<std::size_t>(nodeId) <= stats_.perNode.size()) {
        stats_.perNode[static_cast<std::size_t>(nodeId) - 1].recordsReceived += count;
        stats_.perNode[static_cast<std::size_t>(nodeId) - 1].recordsLoaded += count;
    }
}

ClusterStatistics StatisticsCollector::snapshot() const {
    return stats_;
}

}  // namespace datastorage
