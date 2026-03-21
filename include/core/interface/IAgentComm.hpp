#pragma once
#include <cstdint>
#include <memory>

#include "../message/Message.hpp"

namespace core {
namespace interface {

class IAgentComm {
   public:
    virtual ~IAgentComm() = default;

    virtual void     send(std::shared_ptr<message::Message> msg) = 0;
    virtual uint32_t getAgentId() const                          = 0;
    virtual bool     isHealthy() const                           = 0;
    virtual void     disconnect()                                = 0;

    virtual std::shared_ptr<message::Message> getSetModeMsg(uint32_t mode, uint32_t header_id) = 0;
};

}  // namespace interface
}  // namespace core
