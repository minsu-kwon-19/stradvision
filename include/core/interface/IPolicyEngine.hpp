#pragma once
#include <string>

#include "ICommandBus.hpp"
#include "IStateStore.hpp"

namespace core {
namespace interface {

class IPolicyEngine {
   public:
    virtual ~IPolicyEngine() = default;

    virtual void evaluate(const IStateStore& store, ICommandBus& bus) = 0;
    virtual bool reloadConfig(const std::string& config_path)         = 0;
};

}  // namespace interface
}  // namespace core
