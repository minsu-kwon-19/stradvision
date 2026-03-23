#include "controller/MetricsServer.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>

namespace controller {

class MetricsSession : public std::enable_shared_from_this<MetricsSession> {
   public:
    MetricsSession(asio::ip::tcp::socket socket, std::shared_ptr<MetricsTracker> tracker)
        : socket_(std::move(socket)), tracker_(tracker) {}

    void start() {
        std::ostringstream oss;
        oss << "# HELP stradvision_active_connections Number of active agents connected\n";
        oss << "# TYPE stradvision_active_connections gauge\n";
        oss << "stradvision_active_connections " << tracker_->active_connections.load() << "\n\n";

        oss << "# HELP stradvision_command_failures_total Total command failures (NACK or Timeout)\n";
        oss << "# TYPE stradvision_command_failures_total counter\n";
        oss << "stradvision_command_failures_total " << tracker_->command_failures.load() << "\n\n";

        oss << "# HELP stradvision_avg_rtt_ms Average Round Trip Time in ms\n";
        oss << "# TYPE stradvision_avg_rtt_ms gauge\n";
        uint64_t samples = tracker_->rtt_samples.load();
        uint64_t total   = tracker_->total_rtt_ms.load();
        double   avg_rtt = samples > 0 ? static_cast<double>(total) / samples : 0.0;
        oss << "stradvision_avg_rtt_ms " << avg_rtt << "\n";

        std::string body = oss.str();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/plain; version=0.0.4\r\n"
                 << "Content-Length: " << body.size() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << body;

        response_str_ = response.str();

        auto self = shared_from_this();
        asio::async_write(
            socket_, asio::buffer(response_str_),
            [self](asio::error_code ec, std::size_t /*length*/) {
                asio::error_code ignore_ec;
                self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
                self->socket_.close(ignore_ec);
            });
    }

   private:
    asio::ip::tcp::socket           socket_;
    std::shared_ptr<MetricsTracker> tracker_;
    std::string                     response_str_;
};

MetricsServer::MetricsServer(asio::io_context& ioc, short port, std::shared_ptr<MetricsTracker> tracker)
    : acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), tracker_(tracker) {
    spdlog::info("Metrics Server started on port {}", port);
    doAccept();
}

MetricsServer::~MetricsServer() {
    asio::error_code ec;
    acceptor_.close(ec);
}

void MetricsServer::doAccept() {
    if (!acceptor_.is_open()) return;
    acceptor_.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec) {
            std::make_shared<MetricsSession>(std::move(socket), tracker_)->start();
        }
        doAccept();
    });
}

}  // namespace controller
