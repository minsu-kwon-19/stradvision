#include "core/message/payload/AckPayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

AckPayload::AckPayload(uint32_t id) : cmd_id(id) {
}

std::vector<uint8_t> AckPayload::serialize() const {
    std::vector<uint8_t> buf;
    write_u32_be(buf, cmd_id);
    return buf;
}
void AckPayload::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 4) throw std::runtime_error("Invalid AckPayload size");
    cmd_id = read_u32_be(data.data());
}

}  // namespace message
}  // namespace core
