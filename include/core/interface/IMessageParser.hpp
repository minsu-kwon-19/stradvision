#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "core/message/Message.hpp"

namespace core {
namespace interface {

class IMessageParser {
   public:
    virtual ~IMessageParser() = default;

    virtual std::vector<uint8_t>              serialize(const message::Message& msg) = 0;
    virtual std::shared_ptr<message::Message> deserialize(const std::vector<uint8_t>& data,
                                                          size_t& bytes_consumed)    = 0;
    virtual size_t                            getHeaderSize() const                  = 0;
};

}  // namespace interface
}  // namespace core
