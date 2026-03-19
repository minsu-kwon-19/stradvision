#include "agent/Agent.hpp"

#include <spdlog/spdlog.h>

#include <chrono>

#include "core/message/BinaryMessageParser.hpp"

using namespace core::message;
using namespace core::comm;

namespace agent {

Agent::Agent(asio::io_context& ioc, uint32_t agent_id)
    : ioc_(ioc), resolver_(ioc), socket_(ioc), timer_(ioc), agent_id_(agent_id) {}

void Agent::start(const std::string& host, const std::string& port) {
    auto endpoints = resolver_.resolve(host, port);
    asio::async_connect(
        socket_, endpoints, [this](const asio::error_code& ec, const asio::ip::tcp::endpoint& ep) {
            if (!ec) {
                spdlog::info("Connected to controller at {}:{}", ep.address().to_string(),
                             ep.port());
                conn_ = TcpComm::create(ioc_, std::move(socket_),
                                        std::make_shared<BinaryMessageParser>());
                conn_->setMessageHandler([this](auto /*c*/, auto m) { onMessage(m); });
                conn_->setErrorHandler([this](auto c, auto e) {
                    spdlog::error("Connection error: {}", e.message());
                    conn_->disconnect();
                    std::exit(1);
                });
                conn_->start();
                sendHello();
                startReporting();
            } else {
                spdlog::error("Failed to connect: {}", ec.message());
                std::exit(1);
            }
        });
}

void Agent::sendHello() {
    HelloPayload payload{agent_id_, 1};
    auto         type = MessageType::HELLO;
    auto         msg =
        std::make_shared<Message>(type, payload.serialize(), agent_id_, getNextId(type));
    conn_->send(msg);
}

void Agent::startReporting() {
    timer_.expires_after(std::chrono::seconds(1));
    timer_.async_wait([this](const asio::error_code& ec) {
        if (!ec) {
            std::uniform_real_distribution<float> load_dist(0.0f, 100.0f);
            std::uniform_real_distribution<float> dist(30.0f, 70.0f);
            uint32_t                              mode = current_mode_;

            auto             hb_type = MessageType::HEARTBEAT;
            HeartbeatPayload hb;
            conn_->send(
                std::make_shared<Message>(hb_type, hb.serialize(), agent_id_, getNextId(hb_type)));

            auto         state_type = MessageType::STATE;
            StatePayload state{mode, load_dist(rng_), 45.0f, last_cmd_rc_};
            conn_->send(std::make_shared<Message>(state_type, state.serialize(), agent_id_,
                                                  getNextId(state_type)));

            startReporting();
        }
    });
}

void Agent::onMessage(std::shared_ptr<Message> msg) {
    spdlog::info("Received message type: 0x{:04x} from Agent {}", static_cast<uint16_t>(msg->header.type), msg->header.agent_id);
    if (msg->header.type == MessageType::CMD_SET_MODE) {
        try {
            CmdSetModePayload payload;
            payload.deserialize(msg->payload);
            current_mode_ = payload.mode;
            last_cmd_rc_  = 0;
            auto ack_type = MessageType::ACK;
            AckPayload ack{msg->header.header_id};
            conn_->send(
                std::make_shared<Message>(ack_type, ack.serialize(), agent_id_, getNextId(ack_type)));
        } catch (const std::exception& e) {
            last_cmd_rc_ = -1;
            auto nack_type = MessageType::NACK;
            NackPayload nack{msg->header.header_id, e.what()};
            conn_->send(std::make_shared<Message>(nack_type, nack.serialize(), agent_id_, getNextId(nack_type)));
        }
    } else {
        spdlog::warn("Unknown message type: {}", (int)msg->header.type);
        auto nack_type = MessageType::NACK;
        NackPayload nack{msg->header.header_id, "Unknown message type"};
        conn_->send(std::make_shared<Message>(nack_type, nack.serialize(), agent_id_,
                                              getNextId(nack_type)));
    }
}

}  // namespace agent
