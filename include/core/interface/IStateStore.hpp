#pragma once
#include <cstdint>

#include "../message/Payloads.hpp"

namespace core {
namespace interface {

class IStateStore {
   public:
    virtual ~IStateStore() = default;

    virtual void updateAgentState(uint32_t agent_id, const message::StatePayload& state, uint64_t timestamp) = 0;
    virtual message::StatePayload getAgentState(uint32_t agent_id) const                                 = 0;
    virtual float                  getAggregateLoad() const                                               = 0;
};

}  // namespace interface
}  // namespace core
