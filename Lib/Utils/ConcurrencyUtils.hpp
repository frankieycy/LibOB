#ifndef CONCURRENCY_UTILS_HPP
#define CONCURRENCY_UTILS_HPP
#include <atomic>
#include <cstddef>
#include <vector>

namespace Utils {
namespace Concurrency {
template<typename T, size_t N>
class SpscRingBuffer {
public:
    bool push(const T& item) {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t next = (head + 1) % N;
        if (next == tail_.load(std::memory_order_acquire))
            return false; // full
        buffer_[head] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire))
            return false; // empty
        item = buffer_[tail];
        tail_.store((tail + 1) % N, std::memory_order_release);
        return true;
    }

private:
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
    T buffer_[N];
};
}
}

#endif
