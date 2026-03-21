#pragma once
#include <cstdint>

#include "MessageType.hpp"

namespace core {
namespace message {

struct Header {
    MessageType type;

    uint32_t agent_id;
    uint32_t header_id;
    uint64_t timestamp;
    uint32_t payload_len;
    uint8_t  flags;

    static constexpr size_t size() {
        return sizeof(uint32_t) +  // payload_len
               2 +                 // type (u16 in binary)
               sizeof(uint8_t) +   // flags
               sizeof(uint32_t) +  // agent_id
               sizeof(uint32_t) +  // header_id
               sizeof(uint64_t);   // timestamp
    }
};

}  // namespace message
}  // namespace core
