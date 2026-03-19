#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "Header.hpp"

namespace core {
namespace message {

class Message {
   public:
    Header               header;
    std::vector<uint8_t> payload;
    uint32_t             crc32;

    Message() = default;
    explicit Message(MessageType type, std::vector<uint8_t> payload = {}, uint32_t agent_id = 0,
                     uint32_t header_id = 0, uint8_t flags = 0);
};

}  // namespace message
}  // namespace core
