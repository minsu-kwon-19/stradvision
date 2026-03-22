#include <spdlog/spdlog.h>

#include <iostream>

#include "agent/Agent.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: agent <host> <port> <agent_id>\n";
            return 1;
        }

        std::string host     = argv[1];
        std::string port     = argv[2];
        uint32_t    agent_id = std::stoul(argv[3]);

        spdlog::set_pattern(
            "{\"time\": \"%Y-%m-%dT%H:%M:%S.%f%z\", \"level\": \"%l\", \"message\": \"%v\"}");
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Starting Agent {} targeting {}:{}", agent_id, host, port);

        asio::io_context ioc;
        agent::Agent     app(ioc, agent_id);
        app.start(host, port);

        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
