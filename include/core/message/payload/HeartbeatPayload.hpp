#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct HeartbeatPayload : public IPayload {
    HeartbeatPayload() = default;

    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
