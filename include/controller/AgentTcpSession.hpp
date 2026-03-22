#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <mutex>

#include "core/comm/TcpComm.hpp"
#include "core/interface/IAgentComm.hpp"
#include "core/message/Message.hpp"
#include "core/message/Payloads.hpp"

namespace controller {

struct PendingCommand {
    std::shared_ptr<core::message::Message> msg;
    std::chrono::steady_clock::time_point   last_send;
    int                                     retry_count = 0;
};

class AgentTcpSession : public core::interface::IAgentComm {
   public:
    explicit AgentTcpSession(std::shared_ptr<core::comm::TcpComm> conn);

    uint32_t getAgentId() const override;
    void     setAgentId(uint32_t id);

    void send(std::shared_ptr<core::message::Message> msg) override;
    bool isHealthy() const override;
    void disconnect() override;
    void updateHeartbeat();
    void flushPendingCommands();

    std::shared_ptr<core::message::Message> getSetModeMsg(uint32_t mode, uint32_t header_id) override;

    void handleAck(uint32_t cmd_id) override;
    void checkCommandTimeouts() override;
    void trackCommand(std::shared_ptr<core::message::Message> msg) override;

   private:
    std::shared_ptr<core::comm::TcpComm>  conn_;
    uint32_t                              id_             = 0;
    std::chrono::steady_clock::time_point last_heartbeat_ = std::chrono::steady_clock::now();

    std::map<uint32_t, PendingCommand> pending_cmds_;
    mutable std::mutex                 mutex_;

    core::message::CmdSetModePayload        payload_set_mode_;
    std::shared_ptr<core::message::Message> msg_set_mode_;
};

}  // namespace controller
