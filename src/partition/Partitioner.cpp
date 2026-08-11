#include "partition/Partitioner.h"

#include <stdexcept>

namespace datastorage {

/**
 * Deterministic hash-based partitioning using FNV-1a hash algorithm.
 * 
 * Algorithm explanation:
 * FNV-1a (Fowler-Noll-Vo) is a fast, non-cryptographic hash function with good distribution.
 * 
 * Process:
 * 1. Start with FNV offset basis: 14695981039346656037 (64-bit prime)
 * 2. For each byte in the key:
 *    a. XOR hash with byte value
 *    b. Multiply by FNV prime: 1099511628211
 * 3. Take modulo nodeCount to assign ownership
 * 
 * Properties:
 * - Deterministic: same key always maps to same node
 * - Uniform distribution: keys spread evenly across nodes
 * - Fast: O(n) where n is key length
 * - Collision-resistant: different keys rarely hash to same value
 * 
 * Memory efficiency: operates on key bytes in-place, no temporary buffers
 */
NodeId ModuloPartitioner::owner(const Record& record, std::int32_t nodeCount) const {
    if (nodeCount <= 0) {
        throw std::runtime_error("nodeCount must be positive for partitioning");
    }
    if (record.key.empty()) {
        throw std::runtime_error("Record key is missing; cannot determine ownership");
    }

    // FNV-1a hash implementation
    std::uint64_t hashValue = 14695981039346656037ull;  // FNV offset basis (64-bit)
    for (std::size_t i = 0; i < record.key.size(); ++i) {
        hashValue ^= static_cast<unsigned char>(record.key[i]);  // XOR with byte
        hashValue *= 1099511628211ull;  // Multiply by FNV prime
    }

    // Modulo mapping to determine owner node
    return static_cast<NodeId>(hashValue % static_cast<std::uint64_t>(nodeCount));
}

}  // namespace datastorage
