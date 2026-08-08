#include "verification/Verification.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace datastorage {

Verification::Verification(const Loader& loader, const IPartitioner& partitioner)
    : loader_(loader),
      partitioner_(partitioner) {
}

VerificationResult Verification::verify() const {
    VerificationResult result;

    const std::vector<std::unique_ptr<IKeyValueStore>>& stores = loader_.stores();
    std::unordered_map<std::string, std::int32_t> ownerByKey;
    std::unordered_set<std::string> seenKeys;

    for (std::size_t nodeIndex = 0; nodeIndex < stores.size(); ++nodeIndex) {
        const std::vector<std::string> keys = stores[nodeIndex]->keys();
        for (const std::string& key : keys) {
            Record record;
            if (!stores[nodeIndex]->get(key, &record)) {
                ++result.missingRecords;
                continue;
            }

            ++result.recordsChecked;
            const NodeId expectedOwner = partitioner_.owner(record, static_cast<std::int32_t>(stores.size()));
            const NodeId actualOwner = static_cast<NodeId>(nodeIndex);

            if (expectedOwner != actualOwner) {
                ++result.incorrectOwner;
            }

            const auto existing = ownerByKey.find(key);
            if (existing != ownerByKey.end()) {
                ++result.duplicateOwners;
            } else {
                ownerByKey[key] = actualOwner;
            }

            if (seenKeys.find(key) != seenKeys.end()) {
                ++result.duplicateOwners;
            } else {
                seenKeys.insert(key);
            }
        }
    }

    result.passed = result.incorrectOwner == 0u && result.missingRecords == 0u && result.duplicateOwners == 0u;
    return result;
}

}  // namespace datastorage
