#ifndef CONCURRENCY_UTILS_HPP
#define CONCURRENCY_UTILS_HPP
#include <atomic>
#include <cstddef>
#include <vector>

namespace Utils {
namespace Concurrency {
template<typename T>
class SpscPreallocatedBuffer {
public:
    explicit SpscPreallocatedBuffer() = default;
    explicit SpscPreallocatedBuffer(size_t capacity) : buffer_(capacity) {}

    void reserve(size_t capacity) {
        buffer_.reserve(capacity);
    }

    void push(T&& item) {
        size_t idx = write_.fetch_add(1, std::memory_order_relaxed);
        buffer_[idx] = std::move(item);
    }

    bool try_pop(T& out) {
        if (read_ == write_.load(std::memory_order_acquire))
            return false;
        out = std::move(buffer_[read_++]);
        return true;
    }

private:
    std::vector<T> buffer_;
    std::atomic<size_t> write_{0};
    size_t read_{0}; // consumer only
};

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
