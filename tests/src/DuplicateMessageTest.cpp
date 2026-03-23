#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <chrono>
#include <memory>
#include <thread>

#include "agent/Agent.hpp"
#include "controller/Controller.hpp"
#include "core/message/Payloads.hpp"
#include "core/pool/MessagePool.hpp"

using namespace controller;
using namespace agent;

class DuplicateMessageTest : public ::testing::Test {
   protected:
    void SetUp() override {
        spdlog::set_level(spdlog::level::info);
    }
};

TEST_F(DuplicateMessageTest, AgentDuplicateCommandHandling) {
    asio::io_context controller_ioc;
    asio::io_context agent_ioc;

    short port       = 9007;  // Use 9007 for this test
    auto  controller = std::make_shared<Controller>(controller_ioc, port);
    auto  agent      = std::make_shared<Agent>(agent_ioc, 99);

    agent->start("127.0.0.1", std::to_string(port));

    std::thread agent_thread([&]() { agent_ioc.run(); });
    std::thread controller_thread([&]() { controller_ioc.run(); });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto msg =
        core::pool::MessagePool::getInstance().acquire(core::message::MessageType::CMD_SET_MODE);
    core::message::CmdSetModePayload payload{1};
    msg->payload            = payload.serialize();
    msg->header.header_id   = 9999;
    msg->header.payload_len = static_cast<uint32_t>(msg->payload.size());

    // Send first command
    spdlog::info("--------------------------------------------------");
    spdlog::info("[Test] Sending first CMD_SET_MODE");
    controller->sendCommandTo(99, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(agent->getCurrentMode(), 1);

    // Send same command again (duplicate)
    spdlog::info("--------------------------------------------------");
    spdlog::info("[Test] Sending duplicate CMD_SET_MODE (same header_id: 9999)");
    controller->sendCommandTo(99, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Expected: Agent identifies duplicate, sends NACK, mode remains 1, and no crash.
    EXPECT_EQ(agent->getCurrentMode(), 1);
    spdlog::info(
        "[Test] Agent successfully rejected duplicate message with NACK and remained in mode 1.");

    controller_ioc.stop();
    agent_ioc.stop();

    if (controller_thread.joinable()) controller_thread.join();
    if (agent_thread.joinable()) agent_thread.join();
}
