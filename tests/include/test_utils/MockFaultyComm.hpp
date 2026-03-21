#pragma once

#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <functional>
#include <memory>

#include "core/comm/TcpComm.hpp"
#include "core/message/Message.hpp"
#include "core/message/Payloads.hpp"

namespace test {
namespace utils {

class MockFaultyComm : public core::comm::TcpComm {
   public:
    static std::shared_ptr<MockFaultyComm> create(asio::io_context& ioc);
    explicit MockFaultyComm(asio::io_context& ioc);

    void send(std::shared_ptr<core::message::Message> msg) override;
    void setMessageHandler(MessageHandler handler) override;

    bool isConnected() const override;
    void start() override;
    void disconnect() override;

   private:
    MessageHandler handler_;
    int            receive_count_    = 0;
    uint32_t       last_received_id_ = 0;
};

}  // namespace utils
}  // namespace test
