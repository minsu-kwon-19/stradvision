#pragma once

#include <asio.hpp>
#include <map>
#include <memory>
#include <random>
#include <string>

#include "core/comm/TcpComm.hpp"
#include "core/message/Payloads.hpp"

namespace agent {

class Agent {
   public:
    explicit Agent(asio::io_context& ioc, uint32_t agent_id);
    void     start(const std::string& host, const std::string& port);

   private:
    void sendHello();
    void startReporting();
    void onMessage(std::shared_ptr<core::message::Message> msg);

    uint32_t getNextId(core::message::MessageType type) {
        return message_counters_[type]++;
    }

    asio::io_context&       ioc_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket   socket_;
    asio::steady_timer      timer_;

    std::shared_ptr<core::comm::TcpComm> conn_;

    uint32_t agent_id_;
    uint32_t current_mode_ = 0;
    int32_t  last_cmd_rc_  = 0;

    std::mt19937 rng_{std::random_device{}()};
    std::map<core::message::MessageType, uint32_t> message_counters_;
};

}  // namespace agent
