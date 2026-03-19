#include "core/message/Message.hpp"

#include <atomic>
#include <chrono>

namespace core {
namespace message {

Message::Message(MessageType type, std::vector<uint8_t> payload, uint32_t agent_id,
                 uint32_t header_id, uint8_t flags)
    : payload(std::move(payload)) {
    header.type        = type;
    header.agent_id    = agent_id;
    header.header_id   = header_id;
    header.flags       = flags;
    header.payload_len = static_cast<uint32_t>(this->payload.size());
    header.timestamp =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count());
    crc32 = 0;
}

}  // namespace message
}  // namespace core
