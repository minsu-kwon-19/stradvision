#include "agent/Agent.hpp"

#include <spdlog/spdlog.h>

#include <chrono>

#include "core/message/BinaryMessageParser.hpp"
#include "core/pool/MessagePool.hpp"

using namespace core::message;
using namespace core::comm;
using namespace core::pool;

namespace agent {

Agent::Agent(asio::io_context& ioc, uint32_t agent_id)
    : ioc_(ioc),
      resolver_(ioc),
      socket_(ioc),
      timer_(ioc),
      retry_timer_(ioc),
      signals_(ioc, SIGINT, SIGTERM),
      agent_id_(agent_id) {
    msg_hello_ = MessagePool::getInstance().acquire(MessageType::HELLO);
    msg_hb_    = MessagePool::getInstance().acquire(MessageType::HEARTBEAT);
    msg_state_ = MessagePool::getInstance().acquire(MessageType::STATE);
    msg_ack_   = MessagePool::getInstance().acquire(MessageType::ACK);
    msg_nack_  = MessagePool::getInstance().acquire(MessageType::NACK);
    handleSignals();
}

void Agent::start(const std::string& host, const std::string& port) {
    host_ = host;
    port_ = port;

    auto endpoints = resolver_.resolve(host, port);
    asio::async_connect(
        socket_, endpoints, [this](const asio::error_code& ec, const asio::ip::tcp::endpoint& ep) {
            if (!ec) {
                spdlog::info("Connected to controller at {}:{}", ep.address().to_string(),
                             ep.port());
                retry_count_ = 0;
                conn_        = TcpComm::create(ioc_, std::move(socket_),
                                               std::make_shared<BinaryMessageParser>());
                conn_->setMessageHandler([this](auto /*c*/, auto m) { onMessage(m); });
                conn_->setErrorHandler(
                    [this](auto /*c*/, auto e) { handleConnectionError(e.message()); });
                conn_->start();
                sendHello();
                startReporting();
            } else {
                handleConnectionError(ec.message());
            }
        });
}

void Agent::handleConnectionError(const std::string& msg) {
    if (conn_) {
        conn_->disconnect();
        conn_.reset();
    }
    socket_.close();

    if (retry_count_ < MAX_RETRIES) {
        retry_count_++;
        spdlog::warn("Connection failed: {}. Retrying in 5s... ({}/{})", msg, retry_count_,
                     MAX_RETRIES);
        timer_.expires_after(std::chrono::seconds(5));
        timer_.async_wait([this](const asio::error_code& ec) {
            if (!ec) {
                start(host_, port_);
            }
        });
    } else {
        spdlog::error("Connection failed: {}. Max retries ({}) reached. Giving up.", msg,
                      MAX_RETRIES);
    }
}

void Agent::sendHello() {
    payload_hello_                 = HelloPayload{agent_id_, 1};
    msg_hello_->payload            = payload_hello_.serialize();
    msg_hello_->header.agent_id    = agent_id_;
    msg_hello_->header.header_id   = getNextHeaderId(MessageType::HELLO);
    msg_hello_->header.timestamp   = std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::system_clock::now().time_since_epoch())
                                         .count();
    msg_hello_->header.payload_len = static_cast<uint32_t>(msg_hello_->payload.size());
    conn_->send(msg_hello_);
}

void Agent::sendAck(uint32_t cmd_id) {
    payload_ack_                 = AckPayload{cmd_id};
    msg_ack_->payload            = payload_ack_.serialize();
    msg_ack_->header.agent_id    = agent_id_;
    msg_ack_->header.header_id   = getNextHeaderId(MessageType::ACK);
    msg_ack_->header.timestamp   = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();
    msg_ack_->header.payload_len = static_cast<uint32_t>(msg_ack_->payload.size());
    if (conn_) conn_->send(msg_ack_);
}

void Agent::sendNack(uint32_t cmd_id, const std::string& reason) {
    payload_nack_                 = NackPayload{cmd_id, reason};
    msg_nack_->payload            = payload_nack_.serialize();
    msg_nack_->header.agent_id    = agent_id_;
    msg_nack_->header.header_id   = getNextHeaderId(MessageType::NACK);
    msg_nack_->header.timestamp   = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count();
    msg_nack_->header.payload_len = static_cast<uint32_t>(msg_nack_->payload.size());
    if (conn_) conn_->send(msg_nack_);
}

void Agent::startReporting() {
    if (is_shutting_down_) return;
    timer_.expires_after(std::chrono::seconds(1));
    timer_.async_wait([this](const asio::error_code& ec) {
        if (!ec) {
            std::uniform_real_distribution<float> load_dist(0.0f, 100.0f);
            uint32_t                              mode = current_mode_;

            // Heartbeat
            msg_hb_->payload            = payload_hb_.serialize();
            msg_hb_->header.agent_id    = agent_id_;
            msg_hb_->header.header_id   = getNextHeaderId(MessageType::HEARTBEAT);
            msg_hb_->header.timestamp   = std::chrono::duration_cast<std::chrono::milliseconds>(
                                              std::chrono::system_clock::now().time_since_epoch())
                                              .count();
            msg_hb_->header.payload_len = static_cast<uint32_t>(msg_hb_->payload.size());
            conn_->send(msg_hb_);

            // State
            payload_state_               = StatePayload{mode, load_dist(rng_), 45.0f, last_cmd_rc_};
            msg_state_->payload          = payload_state_.serialize();
            msg_state_->header.agent_id  = agent_id_;
            msg_state_->header.header_id = getNextHeaderId(MessageType::STATE);
            msg_state_->header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                               std::chrono::system_clock::now().time_since_epoch())
                                               .count();
            msg_state_->header.payload_len = static_cast<uint32_t>(msg_state_->payload.size());
            conn_->send(msg_state_);

            startReporting();
        }
    });
}

void Agent::onMessage(std::shared_ptr<Message> msg) {
    if (msg->header.agent_id == 0) {
        spdlog::info("Received message type: 0x{:04x} from Controller",
                     static_cast<uint16_t>(msg->header.type));
    } else {
        spdlog::info("Received message type: 0x{:04x} from Agent {}",
                     static_cast<uint16_t>(msg->header.type), msg->header.agent_id);
    }

    if (msg->header.type == MessageType::CMD_SET_MODE) {
        if (msg->header.header_id == cmd_set_mode_header_id_) {
            spdlog::warn(
                "Agent {}: Duplicate CMD_SET_MODE received (header_id: {}). Rejecting with NACK.",
                agent_id_, msg->header.header_id);
            sendNack(msg->header.header_id, "Duplicate CMD_SET_MODE command");
            return;
        }

        cmd_set_mode_header_id_ = msg->header.header_id;

        try {
            CmdSetModePayload payload;
            payload.deserialize(msg->payload);
            current_mode_ = payload.mode;
            last_cmd_rc_  = 0;

            sendAck(msg->header.header_id);
        } catch (const std::exception& e) {
            last_cmd_rc_ = -1;
            sendNack(msg->header.header_id, e.what());
        }
    } else {
        spdlog::warn("Unknown message type: {}", (int)msg->header.type);
        sendNack(msg->header.header_id, "Unknown message type");
    }
}

uint32_t Agent::getNextHeaderId(core::message::MessageType type) {
    return message_counters_[type]++;
}

uint32_t Agent::getCurrentMode() const {
    return current_mode_;
}

void Agent::handleSignals() {
    signals_.async_wait([this](asio::error_code ec, int signo) {
        if (!ec) {
            spdlog::info("Agent {}: Received signal {}. Initiating Shutdown...", agent_id_, signo);
            shutdownAgent();
        }
    });
}

void Agent::shutdownAgent() {
    if (is_shutting_down_) return;
    is_shutting_down_ = true;

    timer_.cancel();
    retry_timer_.cancel();

    if (conn_) {
        conn_->flushAndDisconnect();
    }
}

}  // namespace agent
