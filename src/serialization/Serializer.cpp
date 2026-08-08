#include "serialization/Serializer.h"

#include <cstring>
#include <stdexcept>

namespace datastorage {

namespace {

std::string encodeUint32(std::uint32_t value) {
    std::string bytes;
    bytes.resize(sizeof(std::uint32_t));
    for (std::size_t i = 0; i < sizeof(std::uint32_t); ++i) {
        bytes[i] = static_cast<char>((value >> (8 * (sizeof(std::uint32_t) - 1u - i))) & 0xFFu);
    }
    return bytes;
}

std::uint32_t decodeUint32(const std::string& bytes, std::size_t offset) {
    if (offset + sizeof(std::uint32_t) > bytes.size()) {
        throw std::runtime_error("Serialized buffer is too short to decode a length field");
    }

    std::uint32_t value = 0;
    for (std::size_t i = 0; i < sizeof(std::uint32_t); ++i) {
        value = (value << 8) | static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[offset + i]));
    }
    return value;
}

std::string encodeInt32(std::int32_t value) {
    std::uint32_t raw = static_cast<std::uint32_t>(value);
    return encodeUint32(raw);
}

std::int32_t decodeInt32(const std::string& bytes, std::size_t offset) {
    const std::uint32_t raw = decodeUint32(bytes, offset);
    return static_cast<std::int32_t>(raw);
}

}  // namespace

std::string Serializer::encodeLength(std::uint32_t value) {
    return encodeUint32(value);
}

std::uint32_t Serializer::decodeLength(const std::string& buffer, std::size_t* position) {
    if (position == nullptr) {
        throw std::runtime_error("Length decode requires a valid position pointer");
    }

    const std::uint32_t value = decodeUint32(buffer, *position);
    *position += sizeof(std::uint32_t);
    return value;
}

std::string Serializer::serialize(const Record& record) {
    std::string output;

    const std::string keyBytes = record.key;
    const std::uint32_t fieldCount = static_cast<std::uint32_t>(record.fields.size());

    if (record.key.size() > 0xFFFFFFFFu) {
        throw std::runtime_error("Record key is too large to serialize");
    }

    output += encodeLength(static_cast<std::uint32_t>(keyBytes.size()));
    output += keyBytes;
    output += encodeLength(fieldCount);

    for (std::size_t i = 0; i < record.fields.size(); ++i) {
        const FieldValue& field = record.fields[i];
        output += encodeLength(static_cast<std::uint32_t>(field.type == FieldType::String ? 1u : 2u));
        if (field.type == FieldType::String) {
            const std::string valueBytes = field.stringValue;
            output += encodeLength(static_cast<std::uint32_t>(valueBytes.size()));
            output += valueBytes;
        } else if (field.type == FieldType::Int32) {
            const std::string bytes = encodeInt32(field.int32Value);
            output += encodeLength(static_cast<std::uint32_t>(sizeof(std::int32_t)));
            output += bytes;
        } else {
            throw std::runtime_error("Unsupported field type for serialization");
        }
    }

    return output;
}

Record Serializer::deserialize(const std::string& buffer) {
    if (buffer.empty()) {
        throw std::runtime_error("Empty serialized record buffer");
    }

    std::size_t position = 0;
    Record record;

    const std::uint32_t keyLength = decodeLength(buffer, &position);
    if (position + keyLength > buffer.size()) {
        throw std::runtime_error("Serialized record key exceeds buffer bounds");
    }
    record.key.assign(buffer.begin() + static_cast<std::ptrdiff_t>(position),
                      buffer.begin() + static_cast<std::ptrdiff_t>(position + keyLength));
    position += keyLength;

    const std::uint32_t fieldCount = decodeLength(buffer, &position);
    record.fields.resize(fieldCount);

    for (std::uint32_t i = 0; i < fieldCount; ++i) {
        const std::uint32_t typeCode = decodeLength(buffer, &position);
        const std::uint32_t valueLength = decodeLength(buffer, &position);

        FieldValue field;
        if (typeCode == 1u) {
            field.type = FieldType::String;
            if (position + valueLength > buffer.size()) {
                throw std::runtime_error("Serialized string field exceeds buffer bounds");
            }
            field.stringValue.assign(buffer.begin() + static_cast<std::ptrdiff_t>(position),
                                     buffer.begin() + static_cast<std::ptrdiff_t>(position + valueLength));
            position += valueLength;
        } else if (typeCode == 2u) {
            field.type = FieldType::Int32;
            if (position + valueLength > buffer.size()) {
                throw std::runtime_error("Serialized int32 field exceeds buffer bounds");
            }
            if (valueLength != sizeof(std::int32_t)) {
                throw std::runtime_error("Invalid int32 payload length during deserialization");
            }
            field.int32Value = decodeInt32(buffer, position);
            position += valueLength;
        } else {
            throw std::runtime_error("Unsupported field type during deserialization");
        }

        record.fields[i] = field;
    }

    return record;
}

}  // namespace datastorage
