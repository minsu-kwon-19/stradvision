#include "core/interface/IPayload.hpp"

namespace core {
namespace message {

struct StatePayload : public IPayload {
    StatePayload() = default;
    explicit StatePayload(uint32_t m, float l, float temp, int32_t rc);

    uint32_t             mode;
    float                load;
    float                temp;
    int32_t              last_rc;
    std::vector<uint8_t> serialize() const override;
    void                 deserialize(const std::vector<uint8_t>& data) override;
};

}  // namespace message
}  // namespace core
