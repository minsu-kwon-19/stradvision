#pragma once

#include <chrono>
#include <memory>

#include "core/comm/TcpComm.hpp"
#include "core/interface/IAgentComm.hpp"
#include "core/message/Message.hpp"

namespace controller {

class AgentTcpSession : public core::interface::IAgentComm {
   public:
    explicit AgentTcpSession(std::shared_ptr<core::comm::TcpComm> conn);

    uint32_t getAgentId() const override;
    void     setAgentId(uint32_t id);

    void send(std::shared_ptr<core::message::Message> msg) override;
    bool isHealthy() const override;
    void disconnect() override;
    void updateHeartbeat();

   private:
    std::shared_ptr<core::comm::TcpComm>  conn_;
    uint32_t                              id_             = 0;
    std::chrono::steady_clock::time_point last_heartbeat_ = std::chrono::steady_clock::now();
};

}  // namespace controller
