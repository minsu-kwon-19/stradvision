#include <spdlog/spdlog.h>

#include "controller/AgentTcpSession.hpp"
#include "core/pool/MessagePool.hpp"

namespace controller {

using namespace core::pool;
using namespace core::message;

AgentTcpSession::AgentTcpSession(std::shared_ptr<core::comm::TcpComm> conn)
    : conn_(std::move(conn)) {
    msg_set_mode_ = MessagePool::getInstance().acquire(MessageType::HELLO);
}

void AgentTcpSession::send(std::shared_ptr<core::message::Message> msg) {
    if (conn_) {
        conn_->send(msg);
    }
}

uint32_t AgentTcpSession::getAgentId() const {
    return id_;
}

void AgentTcpSession::setAgentId(uint32_t id) {
    id_ = id;
}

bool AgentTcpSession::isHealthy() const {
    auto now             = std::chrono::steady_clock::now();
    bool connection_live = (conn_ && conn_->isConnected());
    bool heartbeat_ok =
        std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat_).count() < 3;
    return connection_live && heartbeat_ok;
}

void AgentTcpSession::disconnect() {
    if (conn_) {
        conn_->disconnect();
        conn_.reset();
    }
}

void AgentTcpSession::flushPendingCommands() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pending_cmds_.empty()) {
        if (conn_) conn_->flushAndDisconnect();
        return;
    }
    spdlog::info("Agent {}: Flushing {} pending commands before shutdown.", id_, pending_cmds_.size());
    for (auto& pair : pending_cmds_) {
        if (conn_) {
            conn_->send(pair.second.msg);
        }
    }
    if (conn_) {
        conn_->flushAndDisconnect();
    }
    pending_cmds_.clear();
}

void AgentTcpSession::updateHeartbeat() {
    last_heartbeat_ = std::chrono::steady_clock::now();
}

std::shared_ptr<Message> AgentTcpSession::getSetModeMsg(uint32_t mode, uint32_t header_id) {
    payload_set_mode_.mode          = mode;
    msg_set_mode_->payload          = payload_set_mode_.serialize();
    msg_set_mode_->header.header_id = header_id;
    msg_set_mode_->header.payload_len =
        static_cast<uint32_t>(msg_set_mode_->payload.size());
    return msg_set_mode_;
}

void AgentTcpSession::handleAck(uint32_t cmd_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = pending_cmds_.find(cmd_id);
    if (it != pending_cmds_.end()) {
        spdlog::info("Agent {}: Command 0x{:04x} (ID: {}) ACKed after {} retries", id_,
                     static_cast<uint16_t>(it->second.msg->header.type), cmd_id,
                     it->second.retry_count);
        pending_cmds_.erase(it);
    }
}

void AgentTcpSession::trackCommand(std::shared_ptr<core::message::Message> msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    PendingCommand pc;
    pc.msg         = msg;
    pc.last_send   = std::chrono::steady_clock::now();
    pc.retry_count = 0;
    pending_cmds_[msg->header.header_id] = pc;
}

void AgentTcpSession::checkCommandTimeouts() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        now         = std::chrono::steady_clock::now();
    const int                   MAX_RETRIES = 5;

    for (auto it = pending_cmds_.begin(); it != pending_cmds_.end();) {
        auto& pc = it->second;
        // Exponential backoff: 1s, 2s, 4s, 8s, 16s
        int wait_sec = (1 << pc.retry_count);
        if (std::chrono::duration_cast<std::chrono::seconds>(now - pc.last_send).count() >=
            wait_sec) {
            if (pc.retry_count < MAX_RETRIES) {
                pc.retry_count++;
                pc.last_send = now;
                spdlog::warn("Agent {}: Resending command 0x{:04x} (ID: {}), attempt {}/{}",
                             id_, static_cast<uint16_t>(pc.msg->header.type),
                             pc.msg->header.header_id, pc.retry_count, MAX_RETRIES);
                if (conn_) {
                    conn_->send(pc.msg);
                }
                ++it;
            } else {
                spdlog::error(
                    "Agent {}: Command 0x{:04x} (ID: {}) failed after {} retries. "
                    "Dropping session.",
                    id_, static_cast<uint16_t>(pc.msg->header.type),
                    pc.msg->header.header_id, MAX_RETRIES);
                it = pending_cmds_.erase(it);
                if (conn_) {
                    conn_->disconnect();
                }
            }
        } else {
            ++it;
        }
    }
}

}  // namespace controller
