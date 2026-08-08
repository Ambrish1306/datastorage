#ifndef DATASTORAGE_SERIALIZER_H
#define DATASTORAGE_SERIALIZER_H

#include <cstdint>
#include <string>
#include <vector>

#include "record/Record.h"

namespace datastorage {

class Serializer {
public:
    static std::string serialize(const Record& record);
    static Record deserialize(const std::string& buffer);

private:
    static std::string encodeLength(std::uint32_t value);
    static std::uint32_t decodeLength(const std::string& buffer, std::size_t* position);
};

}  // namespace datastorage

#endif  // DATASTORAGE_SERIALIZER_H
