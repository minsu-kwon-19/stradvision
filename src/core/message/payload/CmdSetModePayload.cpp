#include "core/message/payload/CmdSetModePayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

CmdSetModePayload::CmdSetModePayload(uint32_t m) : mode(m) {
}

std::vector<uint8_t> CmdSetModePayload::serialize() const {
    std::vector<uint8_t> buf;
    write_u32_be(buf, mode);
    return buf;
}
void CmdSetModePayload::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 4) throw std::runtime_error("Invalid CmdSetModePayload size");
    mode = read_u32_be(data.data());
}

}  // namespace message
}  // namespace core
