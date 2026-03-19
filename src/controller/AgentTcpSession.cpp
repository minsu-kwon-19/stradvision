#include "controller/AgentTcpSession.hpp"

namespace controller {

AgentTcpSession::AgentTcpSession(std::shared_ptr<core::comm::TcpComm> conn)
    : conn_(std::move(conn)) {
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

}  // namespace controller
