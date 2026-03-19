#include "core/message/payload/HeartbeatPayload.hpp"

#include <stdexcept>

#include "core/utils/ByteSerializer.hpp"

namespace core {
namespace message {
using namespace core::utils;

std::vector<uint8_t> HeartbeatPayload::serialize() const {
    return {};
}
void HeartbeatPayload::deserialize(const std::vector<uint8_t>& /*data*/) {
    // No-op
}

}  // namespace message
}  // namespace core
