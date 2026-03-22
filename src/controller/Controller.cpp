#include "controller/Controller.hpp"

#include <spdlog/spdlog.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "core/message/BinaryMessageParser.hpp"
#include "core/message/Payloads.hpp"

using json = nlohmann::json;

using namespace core::message;
using namespace core::comm;

namespace controller {

Controller::Controller(asio::io_context& ioc, short port)
    : ioc_(ioc),
      acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      timer_(ioc),
      signals_(ioc, SIGINT, SIGTERM) {
    loadPolicies();
    doAccept();
    startHealthCheck();
    handleSignals();

    auto endpoint = acceptor_.local_endpoint();

    spdlog::info("Server started - IP: {}, Port: {}", endpoint.address().to_string(),
                 endpoint.port());
}

void Controller::broadcastCommand(std::shared_ptr<Message> msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [id, session] : sessions_) {
        if (session->isHealthy()) {
            session->send(msg);
        }
    }
}

void Controller::sendCommandTo(uint32_t agent_id, std::shared_ptr<Message> msg) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(agent_id);
    if (it != sessions_.end() && it->second->isHealthy()) {
        it->second->send(msg);
    }
}

void Controller::doAccept() {
    if (is_shutting_down_) {
        return;
    }
    acceptor_.async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
        if (!ec) {
            spdlog::info("Accepted new connection");
            auto conn =
                TcpComm::create(ioc_, std::move(socket), std::make_shared<BinaryMessageParser>());

            conn->setMessageHandler([this](auto c, auto m) { onMessage(c, m); });
            conn->setErrorHandler([this](auto c, auto e) {
                spdlog::warn("Connection closed: {}", e.message());
                if (c->getContext().has_value()) {
                    auto s = std::any_cast<std::shared_ptr<AgentTcpSession>>(c->getContext());
                    if (s && s->getAgentId() != 0) {
                        std::lock_guard<std::mutex> lock(mutex_);
                        sessions_.erase(s->getAgentId());
                    }
                }
            });

            conn->start();
        }
        doAccept();
    });
}

void Controller::onMessage(std::shared_ptr<TcpComm> conn, std::shared_ptr<Message> msg) {
    std::shared_ptr<AgentTcpSession> session = nullptr;
    if (conn->getContext().has_value()) {
        session = std::any_cast<std::shared_ptr<AgentTcpSession>>(conn->getContext());
    }

    // Registration step
    if (!session) {
        if (msg->header.type == MessageType::HELLO) {
            HelloPayload payload;
            payload.deserialize(msg->payload);
            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto                        new_session = std::make_shared<AgentTcpSession>(conn);
                new_session->setAgentId(payload.agent_id);
                sessions_[payload.agent_id] = new_session;
                conn->setContext(new_session);
            }
            spdlog::info("Agent {} registered", payload.agent_id);
        } else {
            spdlog::warn("Unregistered connection sent non-HELLO message (type: {}). Dropping.",
                         (int)msg->header.type);
            conn->disconnect();
        }
        return;
    }

    // Hello (Already registered)
    if (msg->header.type == MessageType::HELLO) {
        spdlog::warn("Agent {} sent extra HELLO message. Ignored.", session->getAgentId());
        return;
    }

    // Heartbeat
    if (msg->header.type == MessageType::HEARTBEAT) {
        session->updateHeartbeat();
        // spdlog::debug("Agent {} HEARTBEAT", session->getAgentId());
        return;
    }

    // State
    if (msg->header.type == MessageType::STATE) {
        StatePayload payload;
        payload.deserialize(msg->payload);
        store_.updateAgentState(session->getAgentId(), payload, msg->header.timestamp);

        spdlog::debug("Agent {} STATE (header_id: {}, load: {:.2f}%)", session->getAgentId(),
                      msg->header.header_id, payload.load);
        return;
    }

    // Ack
    if (msg->header.type == MessageType::ACK) {
        AckPayload payload;
        payload.deserialize(msg->payload);
        session->handleAck(payload.cmd_id);
        return;
    }

    // NAck
    if (msg->header.type == MessageType::NACK) {
        NackPayload payload;
        payload.deserialize(msg->payload);
        spdlog::warn("Agent {} NACK (cmd_id: {}): {}", session->getAgentId(), payload.cmd_id,
                     payload.reason);
        session->handleAck(payload.cmd_id);  // Also remove from pending
        return;
    }
}

void Controller::startHealthCheck() {
    if (is_shutting_down_) {
        return;
    }
    timer_.expires_after(std::chrono::seconds(2));
    timer_.async_wait([this](const asio::error_code& ec) {
        if (!ec) {
            checkPolicyUpdate();

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto it = sessions_.begin(); it != sessions_.end();) {
                if (!it->second->isHealthy()) {
                    spdlog::warn("Agent {} is unhealthy, dropping session", it->first);
                    it->second->disconnect();
                    it = sessions_.erase(it);
                } else {
                    it->second->checkCommandTimeouts();
                    ++it;
                }
            }

            float avg_load = store_.getAggregateLoad();
            if (!sessions_.empty()) spdlog::debug("Average Agent Load: {:.2f}%", avg_load);

            for (const auto& policy : policies_) {
                bool condition_met = false;
                if (policy.condition.op == ">") {
                    condition_met = (avg_load > policy.condition.threshold);
                } else if (policy.condition.op == "<") {
                    condition_met = (avg_load < policy.condition.threshold);
                }

                if (condition_met) {
                    if (policy.action.command == MessageType::CMD_SET_MODE) {
                        bool already_in_mode = (overload_mode_ == (policy.action.mode == 1));
                        if (!already_in_mode) {
                            spdlog::info("Policy '{}' triggered. Mode -> {}", policy.name,
                                         policy.action.mode);
                            overload_mode_ = (policy.action.mode == 1);

                            uint32_t next_id = getNextId(MessageType::CMD_SET_MODE);
                            for (auto& [id, s] : sessions_) {
                                if (s->isHealthy()) {
                                    auto msg = s->getSetModeMsg(policy.action.mode, next_id);
                                    s->trackCommand(msg);
                                    s->send(msg);
                                }
                            }
                        }
                    }
                }
            }

            startHealthCheck();
        }
    });
}

void Controller::loadPolicies() {
    std::string path = "configs/policy.json";
    try {
        if (!std::filesystem::exists(path)) {
            spdlog::error("Policy file not found: {}", path);
            return;
        }

        std::ifstream f(path);
        json          data = json::parse(f);

        std::vector<Policy> new_policies;
        for (const auto& p : data["policies"]) {
            Policy policy;
            policy.name                = p["name"];
            policy.condition.metric    = p["condition"]["metric"];
            policy.condition.op        = p["condition"]["operator"];
            policy.condition.threshold = p["condition"]["threshold"];

            std::string cmd_str = p["action"]["command"];
            if (cmd_str == "CMD_SET_MODE") {
                policy.action.command = MessageType::CMD_SET_MODE;
            }
            policy.action.mode = p["action"]["mode"];

            new_policies.push_back(policy);
        }

        for (const auto& policy : new_policies) {
            spdlog::info(
                "  [Policy] {}: {} {} {:.1f} -> {}(mode: {})", policy.name, policy.condition.metric,
                policy.condition.op, policy.condition.threshold,
                (policy.action.command == MessageType::CMD_SET_MODE ? "CMD_SET_MODE" : "UNKNOWN"),
                policy.action.mode);
        }

        policies_              = std::move(new_policies);
        last_policy_file_time_ = std::filesystem::last_write_time(path);
        spdlog::info("Loaded {} policies from {}", policies_.size(), path);
    } catch (const std::exception& e) {
        spdlog::error("Failed to load policies: {}", e.what());
    }
}

void Controller::checkPolicyUpdate() {
    std::string path = "configs/policy.json";
    try {
        if (std::filesystem::exists(path)) {
            auto current_time = std::filesystem::last_write_time(path);
            if (current_time > last_policy_file_time_) {
                spdlog::info("Policy file changed. Reloading...");
                loadPolicies();
            }
        }
    } catch (...) {
    }
}

void Controller::handleSignals() {
    signals_.async_wait([this](asio::error_code ec, int signo) {
        if (!ec) {
            spdlog::info("Received signal {}. Initiating Graceful Shutdown...", signo);
            shutdownController();
        }
    });
}

void Controller::shutdownController() {
    if (is_shutting_down_) {
        return;
    }
    is_shutting_down_ = true;

    asio::error_code ec;
    acceptor_.cancel(ec);
    timer_.cancel();

    spdlog::info("Flushing pending commands for all {} sessions...", sessions_.size());
    for (auto& [id, session] : sessions_) {
        session->flushPendingCommands();
    }
}

}  // namespace controller
