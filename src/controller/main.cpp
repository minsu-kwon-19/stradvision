#include <spdlog/spdlog.h>

#include <iostream>

#include "controller/Controller.hpp"

int main(int argc, char* argv[]) {
    try {
        short port = 9000;
        if (argc > 1) {
            port = static_cast<short>(std::stoi(argv[1]));
        }

        spdlog::set_pattern(
            "{\"time\": \"%Y-%m-%dT%H:%M:%S.%f%z\", \"level\": \"%l\", \"message\": \"%v\"}");
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Starting Controller on port {}", port);

        asio::io_context       ioc;
        controller::Controller ctrl(ioc, port);

        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
