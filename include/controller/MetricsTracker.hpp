#pragma once
#include <atomic>
#include <cstdint>

namespace controller {

struct MetricsTracker {
    std::atomic<int>      active_connections{0};
    std::atomic<uint64_t> command_failures{0};
    std::atomic<uint64_t> total_rtt_ms{0};
    std::atomic<uint64_t> rtt_samples{0};
};

}  // namespace controller
