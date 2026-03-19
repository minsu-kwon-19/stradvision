#include "controller/StateStore.hpp"

namespace controller {

void StateStore::updateAgentState(uint32_t agent_id, const core::message::StatePayload& state, uint64_t /*timestamp*/) {
    std::lock_guard<std::mutex> lock(mutex_);
    states_[agent_id] = state;
}

core::message::StatePayload StateStore::getAgentState(uint32_t agent_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = states_.find(agent_id);
    if (it != states_.end()) return it->second;
    return {};
}

float StateStore::getAggregateLoad() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (states_.empty()) return 0.0f;
    float total = 0.0f;
    for (const auto& [id, state] : states_) {
        total += state.load;
    }
    return total / states_.size();
}

} // namespace controller
