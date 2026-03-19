#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct NackPayload : public IPayload {
    NackPayload() = default;
    explicit NackPayload(uint32_t id, std::string r);

    uint32_t             cmd_id;
    std::string          reason;
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
