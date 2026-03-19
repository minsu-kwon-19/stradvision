#pragma once
#include <cstdint>

namespace core {
namespace message {

enum class MessageType : uint16_t {
    HELLO        = 0x0001,
    HEARTBEAT    = 0x0002,
    STATE        = 0x0003,
    CMD_START    = 0x1001,
    CMD_STOP     = 0x1002,
    CMD_SET_MODE = 0x1003,
    ACK          = 0x2001,
    NACK         = 0x2002
};

}  // namespace message
}  // namespace core
