#pragma once
#include <memory>

#include "../message/Message.hpp"

namespace core {
namespace interface {

class ICommandBus {
   public:
    virtual ~ICommandBus()                                                               = default;
    virtual void broadcastCommand(std::shared_ptr<message::Message> msg)                 = 0;
    virtual void sendCommandTo(uint32_t agent_id, std::shared_ptr<message::Message> msg) = 0;
};

}  // namespace interface
}  // namespace core
