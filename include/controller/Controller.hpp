#pragma once

#include <asio.hpp>
#include <asio/signal_set.hpp>
#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "controller/AgentTcpSession.hpp"
#include "controller/StateStore.hpp"
#include "controller/policy/Policy.hpp"
#include "controller/MetricsTracker.hpp"
#include "controller/MetricsServer.hpp"
#include "core/interface/IAgentComm.hpp"
#include "core/interface/ICommandBus.hpp"
#include "core/message/MessageType.hpp"

namespace controller {

class Controller : public core::interface::ICommandBus {
   public:
    explicit Controller(asio::io_context& ioc, short port);

    void broadcastCommand(std::shared_ptr<core::message::Message> msg) override;
    void sendCommandTo(uint32_t agent_id, std::shared_ptr<core::message::Message> msg) override;

    core::interface::IStateStore& getStore() {
        return store_;
    }

   private:
    void doAccept();
    void onMessage(std::shared_ptr<core::comm::TcpComm>    conn,
                   std::shared_ptr<core::message::Message> msg);
    void startHealthCheck();

    void loadPolicies();
    void checkPolicyUpdate();

    void handleSignals();
    void shutdownController();

    uint32_t getNextId(core::message::MessageType type) {
        return message_counters_[type]++;
    }

    asio::io_context&       ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::steady_timer      timer_;
    asio::signal_set        signals_;
    std::mutex              mutex_;
    StateStore              store_;

    std::vector<Policy>                                      policies_;
    std::filesystem::file_time_type                          last_policy_file_time_;
    std::unordered_map<core::message::MessageType, uint32_t> message_counters_;
    bool                                                     overload_mode_ = false;
    bool                                                     is_shutting_down_ = false;

    std::shared_ptr<MetricsTracker> metrics_tracker_;
    std::unique_ptr<MetricsServer>  metrics_server_;

    std::unordered_map<uint32_t, std::shared_ptr<core::interface::IAgentComm>> sessions_;
};

}  // namespace controller
