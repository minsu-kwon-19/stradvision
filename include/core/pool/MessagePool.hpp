#pragma once
#include <memory>
#include <mutex>
#include <vector>

#include "core/message/Message.hpp"

namespace core {
namespace pool {

class MessagePool {
   public:
    static MessagePool& getInstance() {
        static MessagePool instance;
        return instance;
    }

    template <typename... Args>
    std::shared_ptr<message::Message> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.empty()) {
            message::Message* ptr = new message::Message(std::forward<Args>(args)...);
            return std::shared_ptr<message::Message>(
                ptr, [](message::Message* p) { MessagePool::getInstance().release(p); });
        } else {
            message::Message* ptr = pool_.back();
            pool_.pop_back();
            ptr->~Message();
            new (ptr) message::Message(std::forward<Args>(args)...);
            return std::shared_ptr<message::Message>(
                ptr, [](message::Message* p) { MessagePool::getInstance().release(p); });
        }
    }

    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

   private:
    MessagePool() = default;
    ~MessagePool() {
        for (message::Message* ptr : pool_) {
            delete ptr;
        }
    }

    void release(message::Message* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push_back(ptr);
    }

    std::vector<message::Message*> pool_;
    mutable std::mutex             mutex_;
};

}  // namespace memory
}  // namespace core
