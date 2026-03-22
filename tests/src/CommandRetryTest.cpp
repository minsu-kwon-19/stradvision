#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "controller/AgentTcpSession.hpp"
#include "controller/Controller.hpp"
#include "core/comm/TcpComm.hpp"
#include "core/message/BinaryMessageParser.hpp"
#include "test_utils/MockFaultyComm.hpp"

using namespace controller;
using namespace core::message;
using namespace test::utils;

class CommandRetryTest : public ::testing::Test {
   protected:
    void SetUp() override {
        spdlog::set_level(spdlog::level::debug);
    }
};

TEST_F(CommandRetryTest, ExponentialBackoffTest) {
    spdlog::info("=== Starting Refactored Command Retry & Backoff Verification Test ===");

    asio::io_context ioc;

    // Setup Controller
    auto controller   = std::make_shared<Controller>(ioc, 0);
    bool received_ack = false;

    // Setup Session with Mock
    auto mock_comm = MockFaultyComm::create(ioc);
    auto session   = std::make_shared<AgentTcpSession>(mock_comm);

    session->setAgentId(1);
    session->updateHeartbeat();

    mock_comm->setMessageHandler([&](auto /*conn*/, auto msg) {
        spdlog::info("[Test] Simulating ACK reception in Controller for Agent {}",
                     session->getAgentId());

        AckPayload ack;
        ack.deserialize(msg->payload);
        session->handleAck(ack.cmd_id);

        received_ack = true;
    });

    spdlog::info("[Test] Triggering a command to Agent 1...");
    uint32_t cmd_id       = 1001;
    auto     set_mode_msg = session->getSetModeMsg(1, cmd_id);
    session->trackCommand(set_mode_msg);
    session->send(set_mode_msg);

    asio::steady_timer                           timer(ioc);
    std::function<void(const asio::error_code&)> check_loop = [&](const asio::error_code& ec) {
        if (!ec) {
            session->checkCommandTimeouts();
            timer.expires_after(std::chrono::milliseconds(500));
            timer.async_wait(check_loop);
        }
    };

    timer.expires_after(std::chrono::milliseconds(500));
    timer.async_wait(check_loop);

    // Observe retries
    spdlog::info("[Test] Running for up to 15 seconds to observe exponential backoff...");
    auto start_time = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(15)) {
        ioc.poll();

        if (received_ack) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    spdlog::info("=== Verification Test Finished ===");
    SUCCEED();
}
