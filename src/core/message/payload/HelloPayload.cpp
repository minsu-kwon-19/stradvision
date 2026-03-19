#include "core/message/payload/HelloPayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

HelloPayload::HelloPayload(uint32_t id, uint32_t v) : agent_id(id), version(v) {
}

std::vector<uint8_t> HelloPayload::serialize() const {
    std::vector<uint8_t> buf;
    write_u32_be(buf, agent_id);
    write_u32_be(buf, version);
    return buf;
}
void HelloPayload::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 8) throw std::runtime_error("Invalid HelloPayload size");
    agent_id = read_u32_be(data.data());
    version  = read_u32_be(data.data() + 4);
}

}  // namespace message
}  // namespace core
