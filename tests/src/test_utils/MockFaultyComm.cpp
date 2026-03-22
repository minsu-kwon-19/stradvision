#include "test_utils/MockFaultyComm.hpp"

namespace test {
namespace utils {

using namespace core::message;

std::shared_ptr<MockFaultyComm> MockFaultyComm::create(asio::io_context& ioc) {
    return std::make_shared<MockFaultyComm>(ioc);
}

MockFaultyComm::MockFaultyComm(asio::io_context& ioc)
    : core::comm::TcpComm(ioc, asio::ip::tcp::socket(ioc), nullptr) {
}

void MockFaultyComm::send(std::shared_ptr<Message> msg) {
    spdlog::info("[MockComm] Received command 0x{:04x} (ID: {})",
                 static_cast<uint16_t>(msg->header.type), msg->header.header_id);

    last_received_id_ = msg->header.header_id;
    receive_count_++;

    // Simulate success only on the 4th attempt
    if (receive_count_ >= 4) {
        spdlog::info("[MockComm] SUCCESS! Queueing ACK for ID: {}", last_received_id_);
        if (handler_) {
            auto       ack_msg = std::make_shared<Message>(MessageType::ACK);
            AckPayload payload{last_received_id_};
            ack_msg->payload         = payload.serialize();
            ack_msg->header.agent_id = 1;  // From Agent 1

            // Defer execution to avoid locking deadlocks in synchronous tests
            asio::post(socket().get_executor(), [this, ack_msg]() {
                handler_(shared_from_this(), ack_msg);
            });
        }
    } else {
        spdlog::warn("[MockComm] Ignoring command (Attempt {}/4)", receive_count_);
    }
}

void MockFaultyComm::setMessageHandler(MessageHandler handler) {
    handler_ = handler;
}

bool MockFaultyComm::isConnected() const {
    return true;
}

void MockFaultyComm::start() {
}

void MockFaultyComm::disconnect() {
}

}  // namespace utils
}  // namespace test
