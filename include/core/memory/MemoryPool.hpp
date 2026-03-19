#pragma once
#include <vector>
#include <mutex>
#include <memory>

namespace core {
namespace memory {

template <typename T>
class MemoryPool {
public:
    static MemoryPool& getInstance() {
        static MemoryPool instance;
        return instance;
    }

    template <typename... Args>
    std::shared_ptr<T> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.empty()) {
            T* ptr = new T(std::forward<Args>(args)...);
            return std::shared_ptr<T>(ptr, [](T* p) {
                MemoryPool::getInstance().release(p);
            });
        } else {
            T* ptr = pool_.back();
            pool_.pop_back();
            ptr->~T();
            new (ptr) T(std::forward<Args>(args)...);
            return std::shared_ptr<T>(ptr, [](T* p) {
                MemoryPool::getInstance().release(p);
            });
        }
    }

    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

private:
    MemoryPool() = default;
    ~MemoryPool() {
        for (T* ptr : pool_) {
            delete ptr;
        }
    }

    void release(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push_back(ptr);
    }

    std::vector<T*> pool_;
    mutable std::mutex mutex_;
};

} // namespace memory
} // namespace core
