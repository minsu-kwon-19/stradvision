#pragma once

#include <string>

#include "core/message/MessageType.hpp"

namespace controller {

struct PolicyCondition {
    std::string metric;
    std::string op;
    float       threshold;
};

struct PolicyAction {
    core::message::MessageType command;
    uint32_t                   mode;
};

struct Policy {
    std::string     name;
    PolicyCondition condition;
    PolicyAction    action;
};

}  // namespace controller
