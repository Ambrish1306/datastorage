#include "partition/Partitioner.h"

#include <stdexcept>

namespace datastorage {

NodeId ModuloPartitioner::owner(const Record& record, std::int32_t nodeCount) const {
    if (nodeCount <= 0) {
        throw std::runtime_error("nodeCount must be positive for partitioning");
    }
    if (record.key.empty()) {
        throw std::runtime_error("Record key is missing; cannot determine ownership");
    }

    std::uint64_t hashValue = 14695981039346656037ull;
    for (std::size_t i = 0; i < record.key.size(); ++i) {
        hashValue ^= static_cast<unsigned char>(record.key[i]);
        hashValue *= 1099511628211ull;
    }

    return static_cast<NodeId>(hashValue % static_cast<std::uint64_t>(nodeCount));
}

}  // namespace datastorage
