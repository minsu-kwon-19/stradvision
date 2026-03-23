#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <chrono>
#include <memory>
#include <thread>

#include "agent/Agent.hpp"
#include "controller/Controller.hpp"

using namespace controller;
using namespace agent;

class IntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        spdlog::set_level(spdlog::level::info);
    }
};

TEST_F(IntegrationTest, ControllerPolicyTriggerTest) {
    asio::io_context controller_ioc;
    asio::io_context agent_ioc;

    const int NUM_AGENTS = 3;
    short     port       = 9005;

    auto controller = std::make_shared<Controller>(controller_ioc, port);

    std::vector<std::shared_ptr<Agent>> agents;
    for (int i = 1; i <= NUM_AGENTS; ++i) {
        agents.push_back(std::make_shared<Agent>(agent_ioc, i));
    }

    std::thread controller_thread([&]() { controller_ioc.run(); });
    std::thread agent_thread([&]() { agent_ioc.run(); });

    for (auto& agent : agents) {
        agent->start("127.0.0.1", std::to_string(port));
    }

    controller->clearPolicies();
    Policy p1;
    p1.name                = "ForceMode1";
    p1.condition.metric    = "average_load";
    p1.condition.op        = ">";
    p1.condition.threshold = -1.0;
    p1.action.command      = core::message::MessageType::CMD_SET_MODE;
    p1.action.mode         = 1;
    controller->addPolicy(p1);

    auto start_time     = std::chrono::steady_clock::now();
    bool phase1_success = false;
    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(15)) {
        int count = 0;
        for (auto& agent : agents) {
            if (agent->getCurrentMode() == 1) count++;
        }
        if (count == NUM_AGENTS) {
            phase1_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    EXPECT_TRUE(phase1_success) << "Timeout waiting for all agents to switch to Mode 1";

    controller->clearPolicies();
    Policy p0;
    p0.name                = "ForceMode0";
    p0.condition.metric    = "average_load";
    p0.condition.op        = ">";
    p0.condition.threshold = -1.0;
    p0.action.command      = core::message::MessageType::CMD_SET_MODE;
    p0.action.mode         = 0;
    controller->addPolicy(p0);

    start_time          = std::chrono::steady_clock::now();
    bool phase2_success = false;
    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(15)) {
        int count = 0;
        for (auto& agent : agents) {
            if (agent->getCurrentMode() == 0) count++;
        }
        if (count == NUM_AGENTS) {
            phase2_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    EXPECT_TRUE(phase2_success) << "Timeout waiting for all agents to recover to Mode 0";

    controller_ioc.stop();
    agent_ioc.stop();

    if (controller_thread.joinable()) {
        controller_thread.join();
    }

    if (agent_thread.joinable()) {
        agent_thread.join();
    }
}
