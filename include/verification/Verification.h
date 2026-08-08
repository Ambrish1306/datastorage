#ifndef DATASTORAGE_VERIFICATION_H
#define DATASTORAGE_VERIFICATION_H

#include <cstddef>
#include <vector>

#include "loader/Loader.h"
#include "partition/Partitioner.h"
#include "storage/Store.h"

namespace datastorage {

struct VerificationResult {
    std::size_t recordsChecked = 0;
    std::size_t incorrectOwner = 0;
    std::size_t missingRecords = 0;
    std::size_t duplicateOwners = 0;
    bool passed = false;
};

class Verification {
public:
    Verification(const Loader& loader, const IPartitioner& partitioner);

    VerificationResult verify() const;

private:
    const Loader& loader_;
    const IPartitioner& partitioner_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_VERIFICATION_H
