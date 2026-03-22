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

    auto start_time  = std::chrono::steady_clock::now();
    bool all_changed = false;

    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(30)) {
        int changed_count = 0;
        for (auto& agent : agents) {
            if (agent->getCurrentMode() == 1) {
                changed_count++;
            }
        }

        if (changed_count >= agents.size()) {
            all_changed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    EXPECT_TRUE(all_changed) << "Not all agents received the mode change within the timeout.";

    controller_ioc.stop();
    agent_ioc.stop();

    if (controller_thread.joinable()) {
        controller_thread.join();
    }

    if (agent_thread.joinable()) {
        agent_thread.join();
    }
}
