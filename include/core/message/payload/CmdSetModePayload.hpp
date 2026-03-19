#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct CmdSetModePayload : public IPayload {
    CmdSetModePayload() = default;
    explicit CmdSetModePayload(uint32_t m);

    uint32_t             mode;
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
