#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct AckPayload : public IPayload {
    AckPayload() = default;
    explicit AckPayload(uint32_t id);

    uint32_t             cmd_id;
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
