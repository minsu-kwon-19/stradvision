#pragma once

#include <asio.hpp>
#include <asio/signal_set.hpp>
#include <chrono>
#include <random>
#include <string>
#include <unordered_map>

#include "core/comm/TcpComm.hpp"
#include "core/message/Payloads.hpp"

namespace agent {

class Agent {
   public:
    explicit Agent(asio::io_context& ioc, uint32_t agent_id);
    void start(const std::string& host, const std::string& port);

    uint32_t getCurrentMode() const;

   private:
    void sendHello();
    void startReporting();
    void startCommandRetryTimer();
    void sendAck(uint32_t cmd_id);
    void sendNack(uint32_t cmd_id, const std::string& reason);

    void handleSignals();
    void shutdownAgent();
    void onMessage(std::shared_ptr<core::message::Message> msg);

    void handleConnectionError(const std::string& msg);

    uint32_t getNextHeaderId(core::message::MessageType type);

    asio::io_context&       ioc_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket   socket_;
    std::shared_ptr<core::comm::TcpComm> conn_;
    asio::steady_timer                   timer_;
    asio::steady_timer                   retry_timer_;
    asio::signal_set                     signals_;

    uint32_t agent_id_;
    uint32_t current_mode_ = 0;
    float    cpu_load_ = 0.0f;
    float    temperature_ = 0.0f;
    int32_t  last_cmd_rc_ = 0;

    bool is_shutting_down_ = false;

    std::string host_;
    std::string port_;

    int       retry_count_ = 0;
    const int MAX_RETRIES  = 3;

    uint32_t cmd_set_mode_header_id_ = 0xFFFFFFFF;

    core::message::HelloPayload     payload_hello_;
    core::message::HeartbeatPayload payload_hb_;
    core::message::StatePayload     payload_state_;
    core::message::AckPayload       payload_ack_;
    core::message::NackPayload      payload_nack_;

    std::shared_ptr<core::message::Message> msg_hello_;
    std::shared_ptr<core::message::Message> msg_hb_;
    std::shared_ptr<core::message::Message> msg_state_;
    std::shared_ptr<core::message::Message> msg_ack_;
    std::shared_ptr<core::message::Message> msg_nack_;

    std::mt19937 rng_{std::random_device{}()};

    std::unordered_map<core::message::MessageType, uint32_t> message_counters_;
};

}  // namespace agent
