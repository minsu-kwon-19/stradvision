#pragma once

#include <mutex>
#include <unordered_map>

#include "core/interface/IStateStore.hpp"
#include "core/message/Payloads.hpp"


namespace controller {

class StateStore : public core::interface::IStateStore {
   public:
    void updateAgentState(uint32_t agent_id, const core::message::StatePayload& state,
                          uint64_t timestamp) override;
    core::message::StatePayload getAgentState(uint32_t agent_id) const override;
    float                       getAggregateLoad() const override;

   private:
    mutable std::mutex                                        mutex_;
    std::unordered_map<uint32_t, core::message::StatePayload> states_;
};

}  // namespace controller
