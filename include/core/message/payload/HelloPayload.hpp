#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct HelloPayload : public IPayload {
    HelloPayload() = default;
    explicit HelloPayload(uint32_t id, uint32_t v);

    uint32_t             agent_id;
    uint32_t             version;
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
