#ifndef DATASTORAGE_PARTITIONER_H
#define DATASTORAGE_PARTITIONER_H

#include <cstdint>

#include "record/Record.h"

namespace datastorage {

using NodeId = std::int32_t;

class IPartitioner {
public:
    virtual ~IPartitioner() = default;

    virtual NodeId owner(const Record& record, std::int32_t nodeCount) const = 0;
};

class ModuloPartitioner : public IPartitioner {
public:
    NodeId owner(const Record& record, std::int32_t nodeCount) const override;
};

}  // namespace datastorage

#endif  // DATASTORAGE_PARTITIONER_H
