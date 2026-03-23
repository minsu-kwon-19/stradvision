#pragma once
#include <any>
#include <asio.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "../interface/IMessageParser.hpp"
#include "../message/BinaryMessageParser.hpp"

namespace core {
namespace comm {

class TcpComm : public std::enable_shared_from_this<TcpComm> {
   public:
    explicit TcpComm(asio::io_context& ioc, asio::ip::tcp::socket socket,
                     std::shared_ptr<interface::IMessageParser> msg_parser);
    virtual ~TcpComm() = default;

    using MessageHandler =
        std::function<void(std::shared_ptr<TcpComm>, std::shared_ptr<message::Message>)>;
    using ErrorHandler = std::function<void(std::shared_ptr<TcpComm>, const asio::error_code&)>;

    static std::shared_ptr<TcpComm> create(asio::io_context& ioc, asio::ip::tcp::socket socket,
                                           std::shared_ptr<interface::IMessageParser> msg_parser) {
        return std::make_shared<TcpComm>(ioc, std::move(socket), msg_parser);
    }

    virtual void setMessageHandler(MessageHandler handler);
    virtual void setErrorHandler(ErrorHandler handler);

    virtual void start();
    virtual void send(std::shared_ptr<message::Message> msg);
    virtual void disconnect();
    virtual void flushAndDisconnect();
    virtual bool isConnected() const;

    asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void setContext(std::any ctx) {
        user_context_ = std::move(ctx);
    }
    const std::any& getContext() const {
        return user_context_;
    }

   private:
    void readHeader();
    void readBody(size_t total_frame_size);
    void writePump();

    asio::ip::tcp::socket             socket_;
    std::vector<uint8_t>              read_buffer_;
    std::vector<std::vector<uint8_t>> write_queue_;

    MessageHandler msg_handler_;
    ErrorHandler   err_handler_;

    bool is_shutting_down_ = false;

    std::any                                   user_context_;
    std::shared_ptr<interface::IMessageParser> msg_parser_;
};

}  // namespace comm
}  // namespace core
