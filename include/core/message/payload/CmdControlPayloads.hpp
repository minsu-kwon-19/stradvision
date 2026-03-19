#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct CmdStartPayload : public IPayload {
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>&) override;
};

struct CmdStopPayload : public IPayload {
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>&) override;
};

}  // namespace message
}  // namespace core
