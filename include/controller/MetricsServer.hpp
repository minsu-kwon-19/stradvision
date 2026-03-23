#pragma once
#include <asio.hpp>
#include <memory>
#include <string>
#include "MetricsTracker.hpp"

namespace controller {

class MetricsServer {
   public:
    MetricsServer(asio::io_context& ioc, short port, std::shared_ptr<MetricsTracker> tracker);
    ~MetricsServer();

   private:
    void doAccept();

    asio::ip::tcp::acceptor         acceptor_;
    std::shared_ptr<MetricsTracker> tracker_;
};

}  // namespace controller
