#include "core/comm/TcpComm.hpp"

#include <iostream>

namespace core {
namespace comm {

TcpComm::TcpComm(asio::io_context& ioc, asio::ip::tcp::socket socket,
                 std::shared_ptr<interface::IMessageParser> msg_parser)
    : socket_(std::move(socket)), msg_parser_(msg_parser) {
    read_buffer_.resize(8192);
}

void TcpComm::start() {
    readHeader();
}

void TcpComm::readHeader() {
    auto   self(shared_from_this());
    size_t header_size = msg_parser_->getHeaderSize();
    asio::async_read(socket_, asio::buffer(read_buffer_.data(), header_size),
                     [this, self, header_size](const asio::error_code& ec,
                                               std::size_t /*bytes_transferred*/) {
                         if (!ec) {
                             uint32_t payload_len = (uint32_t(read_buffer_[0]) << 24) |
                                                    (uint32_t(read_buffer_[1]) << 16) |
                                                    (uint32_t(read_buffer_[2]) << 8) |
                                                    uint32_t(read_buffer_[3]);
                             size_t total_size = header_size + payload_len + 4;
                             if (total_size > read_buffer_.size()) {
                                 read_buffer_.resize(total_size);
                             }
                             readBody(total_size);
                         } else {
                             if (err_handler_) err_handler_(self, ec);
                         }
                     });
}

void TcpComm::readBody(size_t total_frame_size) {
    auto   self(shared_from_this());
    size_t header_size = msg_parser_->getHeaderSize();
    asio::async_read(socket_,
                     asio::buffer(read_buffer_.data() + header_size, total_frame_size - header_size),
                     [this, self, total_frame_size](const asio::error_code& ec,
                                                    std::size_t /*bytes_transferred*/) {
                         if (!ec) {
                             std::shared_ptr<message::Message> msg;
                             try {
                                 size_t consumed = 0;
                                 msg             = msg_parser_->deserialize(read_buffer_, consumed);
                                 if (consumed == total_frame_size && msg && msg_handler_) {
                                     msg_handler_(self, msg);
                                 }
                             } catch (const std::exception& e) {
                                 if (err_handler_)
                                     err_handler_(self, asio::error::make_error_code(
                                                            asio::error::invalid_argument));
                                 return;
                             }
                             readHeader();
                         } else {
                             if (err_handler_) err_handler_(self, ec);
                         }
                     });
}

void TcpComm::send(std::shared_ptr<message::Message> msg) {
    auto encoded = msg_parser_->serialize(*msg);
    auto self(shared_from_this());
    asio::post(socket_.get_executor(), [this, self, buf = std::move(encoded)]() mutable {
        bool idle = write_queue_.empty();
        write_queue_.push_back(std::move(buf));
        if (idle) {
            writePump();
        }
    });
}

void TcpComm::writePump() {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(write_queue_.front()),
                      [this, self](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                          if (!ec) {
                              write_queue_.erase(write_queue_.begin());
                              if (!write_queue_.empty()) {
                                  writePump();
                              }
                          } else {
                              if (err_handler_) err_handler_(self, ec);
                          }
                      });
}

void TcpComm::disconnect() {
    auto self(shared_from_this());
    asio::post(socket_.get_executor(), [this, self]() {
        asio::error_code ec;
        socket_.close(ec);
    });
}

void TcpComm::setMessageHandler(MessageHandler handler) {
    msg_handler_ = handler;
}
void TcpComm::setErrorHandler(ErrorHandler handler) {
    err_handler_ = handler;
}

}  // namespace comm
}  // namespace core
