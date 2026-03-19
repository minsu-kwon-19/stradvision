#pragma once
#include "core/interface/IMessageParser.hpp"
#include "core/message/Message.hpp"

namespace core {
namespace message {

class BinaryMessageParser : public interface::IMessageParser {
   public:
    BinaryMessageParser()           = default;
    ~BinaryMessageParser() override = default;

    std::vector<uint8_t>     serialize(const Message& msg) override;
    std::shared_ptr<Message> deserialize(const std::vector<uint8_t>& data,
                                         size_t&                     bytes_consumed) override;
    size_t                   getHeaderSize() const override;

   private:
    static uint32_t compute_crc32(const uint8_t* data, size_t length);
};

}  // namespace message
}  // namespace core
