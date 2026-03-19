#include "core/message/payload/NackPayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

NackPayload::NackPayload(uint32_t id, std::string r) : cmd_id(id), reason(std::move(r)) {
}

std::vector<uint8_t> NackPayload::serialize() const {
    std::vector<uint8_t> buf;
    write_u32_be(buf, cmd_id);
    buf.insert(buf.end(), reason.begin(), reason.end());
    return buf;
}
void NackPayload::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 4) throw std::runtime_error("Invalid NackPayload size");
    cmd_id = read_u32_be(data.data());
    if (data.size() > 4) {
        reason.assign(data.begin() + 4, data.end());
    }
}

}  // namespace message
}  // namespace core
