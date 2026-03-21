#include "controller/AgentTcpSession.hpp"
#include "core/pool/MessagePool.hpp"

namespace controller {

using namespace core::pool;
using namespace core::message;

AgentTcpSession::AgentTcpSession(std::shared_ptr<core::comm::TcpComm> conn)
    : conn_(std::move(conn)) {
    msg_set_mode_ = MessagePool::getInstance().acquire(MessageType::CMD_SET_MODE);
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
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat_).count() < 3;
}

void AgentTcpSession::disconnect() {
    if (conn_) {
        conn_->disconnect();
        conn_.reset();
    }
}

void AgentTcpSession::updateHeartbeat() {
    last_heartbeat_ = std::chrono::steady_clock::now();
}

std::shared_ptr<Message> AgentTcpSession::getSetModeMsg(uint32_t mode, uint32_t header_id) {
    payload_set_mode_.mode = mode;
    msg_set_mode_->payload = payload_set_mode_.serialize();
    msg_set_mode_->header.header_id = header_id;
    msg_set_mode_->header.payload_len = static_cast<uint32_t>(msg_set_mode_->payload.size());
    return msg_set_mode_;
}

}  // namespace controller
