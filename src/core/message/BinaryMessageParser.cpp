#include "core/message/BinaryMessageParser.hpp"

#include <cstring>
#include <stdexcept>

#include "core/pool/MessagePool.hpp"
#include "core/message/Message.hpp"
#include "core/utils/ByteSerializer.hpp"

using namespace core::pool;

namespace core {
namespace message {
using namespace core::utils;

uint32_t BinaryMessageParser::compute_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

std::vector<uint8_t> BinaryMessageParser::serialize(const Message& msg) {
    size_t               total_size = Header::size() + msg.payload.size() + 4;
    std::vector<uint8_t> buf;
    buf.reserve(total_size);

    write_u32_be(buf, msg.header.payload_len);
    write_u16_be(buf, static_cast<uint16_t>(msg.header.type));
    buf.push_back(msg.header.flags);
    write_u32_be(buf, msg.header.agent_id);
    write_u32_be(buf, msg.header.header_id);
    write_u64_be(buf, msg.header.timestamp);

    if (!msg.payload.empty()) {
        buf.insert(buf.end(), msg.payload.begin(), msg.payload.end());
    }

    uint32_t computed_crc = compute_crc32(buf.data(), buf.size());
    write_u32_be(buf, computed_crc);

    return buf;
}

std::shared_ptr<Message> BinaryMessageParser::deserialize(const std::vector<uint8_t>& data,
                                                          size_t& bytes_consumed) {
    bytes_consumed = 0;
    if (data.size() < Header::size()) return nullptr;

    const uint8_t* ptr              = data.data();
    uint32_t       payload_len      = read_u32_be(ptr);
    uint32_t       total_frame_size = Header::size() + payload_len + 4;

    if (data.size() < total_frame_size) {
        return nullptr;
    }

    uint32_t type_val  = read_u16_be(ptr + 4);
    uint8_t  flags     = ptr[6];
    uint32_t agent_id  = read_u32_be(ptr + 7);
    uint32_t header_id = read_u32_be(ptr + 11);
    uint64_t timestamp = read_u64_be(ptr + 15);

    std::vector<uint8_t> payload_data;
    if (payload_len > 0) {
        payload_data.assign(ptr + Header::size(), ptr + Header::size() + payload_len);
    }

    uint32_t msg_crc      = read_u32_be(ptr + Header::size() + payload_len);
    uint32_t computed_crc = compute_crc32(ptr, Header::size() + payload_len);
    if (msg_crc != computed_crc) {
        throw std::runtime_error("CRC mismatch");
    }

    auto out_msg = MessagePool::getInstance().acquire(static_cast<MessageType>(type_val),
                                                      std::move(payload_data), 0, 0, flags);
    out_msg->header.agent_id  = agent_id;
    out_msg->header.header_id = header_id;
    out_msg->header.timestamp = timestamp;
    out_msg->crc32            = msg_crc;

    bytes_consumed = total_frame_size;
    return out_msg;
}

size_t BinaryMessageParser::getHeaderSize() const {
    return Header::size();
}

}  // namespace message
}  // namespace core
