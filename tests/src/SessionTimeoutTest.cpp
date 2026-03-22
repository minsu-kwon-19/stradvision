#include <gtest/gtest.h>

#include <asio.hpp>
#include <chrono>
#include <thread>

#include "controller/AgentTcpSession.hpp"
#include "core/message/BinaryMessageParser.hpp"

using namespace controller;

class SessionTimeoutTest : public ::testing::Test {
   protected:
    void SetUp() override {
    }
};

TEST_F(SessionTimeoutTest, IsHealthyInitially) {
    asio::io_context      ioc;
    asio::ip::tcp::socket socket(ioc);
    socket.open(asio::ip::tcp::v4());  // Must open so isConnected() returns true
    auto parser = std::make_shared<core::message::BinaryMessageParser>();
    auto conn   = std::make_shared<core::comm::TcpComm>(ioc, std::move(socket), parser);

    AgentTcpSession session(conn);
    session.setAgentId(1);

    // Initial heartbeat is recorded upon creation, should be healthy immediately
    EXPECT_TRUE(session.isHealthy());
}

TEST_F(SessionTimeoutTest, IsHealthyAfterUpdate) {
    asio::io_context      ioc;
    asio::ip::tcp::socket socket(ioc);
    socket.open(asio::ip::tcp::v4());
    auto parser = std::make_shared<core::message::BinaryMessageParser>();
    auto conn   = std::make_shared<core::comm::TcpComm>(ioc, std::move(socket), parser);

    AgentTcpSession session(conn);

    // Explicitly update heartbeat
    session.updateHeartbeat();
    EXPECT_TRUE(session.isHealthy());
}

TEST_F(SessionTimeoutTest, BecomesUnhealthyAfterTimeout) {
    asio::io_context      ioc;
    asio::ip::tcp::socket socket(ioc);
    socket.open(asio::ip::tcp::v4());
    auto parser = std::make_shared<core::message::BinaryMessageParser>();
    auto conn   = std::make_shared<core::comm::TcpComm>(ioc, std::move(socket), parser);

    AgentTcpSession session(conn);

    EXPECT_TRUE(session.isHealthy());

    // Sleep longer than the 3.0 seconds threshold (use 3.1 seconds)
    std::this_thread::sleep_for(std::chrono::milliseconds(3100));

    EXPECT_FALSE(session.isHealthy());
}

TEST_F(SessionTimeoutTest, CommandTrackingAndAck) {
    asio::io_context      ioc;
    asio::ip::tcp::socket socket(ioc);
    auto                  parser = std::make_shared<core::message::BinaryMessageParser>();
    auto conn = std::make_shared<core::comm::TcpComm>(ioc, std::move(socket), parser);

    AgentTcpSession session(conn);

    auto msg = session.getSetModeMsg(2, 100);
    session.trackCommand(msg);

    EXPECT_NO_THROW({ session.handleAck(100); });
}
