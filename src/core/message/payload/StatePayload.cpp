#include "core/message/payload/StatePayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

StatePayload::StatePayload(uint32_t m, float l, float temp, int32_t rc)
    : mode(m), load(l), temp(temp), last_rc(rc) {
}

std::vector<uint8_t> StatePayload::serialize() const {
    std::vector<uint8_t> buf;
    write_u32_be(buf, mode);
    write_float_be(buf, load);
    write_float_be(buf, temp);
    write_u32_be(buf, static_cast<uint32_t>(last_rc));
    return buf;
}
void StatePayload::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 16) throw std::runtime_error("Invalid StatePayload size");
    mode    = read_u32_be(data.data());
    load    = read_float_be(data.data() + 4);
    temp    = read_float_be(data.data() + 8);
    last_rc = static_cast<int32_t>(read_u32_be(data.data() + 12));
}

}  // namespace message
}  // namespace core
