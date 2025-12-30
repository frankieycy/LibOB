#ifndef COUNTER_UTILS_HPP
#define COUNTER_UTILS_HPP
#include <vector>

namespace Utils {
namespace Counter {
class IdHandlerBase {
public:
    IdHandlerBase() = default;
    IdHandlerBase(const IdHandlerBase& idHandler) = default;
    IdHandlerBase(const bool idLogEnabled) : myIdLogEnabled(idLogEnabled) {}
    virtual ~IdHandlerBase() = default;
    virtual std::shared_ptr<IdHandlerBase> clone() const { return std::make_shared<IdHandlerBase>(*this); }
    const std::vector<uint64_t>& getIdLog() const { return myIdLog; }
    uint64_t getCurrentId() const { return myCurrentId; }
    uint64_t generateId();
    void reset();
private:
    uint64_t myCurrentId = 0;
    bool myIdLogEnabled = false;
    std::vector<uint64_t> myIdLog;
};

class TimestampHandlerBase {
public:
    TimestampHandlerBase() = default;
    TimestampHandlerBase(const TimestampHandlerBase& timestampHandler) = default;
    virtual ~TimestampHandlerBase() = default;
    virtual std::shared_ptr<TimestampHandlerBase> clone() const { return std::make_shared<TimestampHandlerBase>(*this); }
    uint64_t getCurrentTimestamp() const { return myCurrentTimestamp; }
    uint64_t tick(const uint64_t elapsedTimeUnit = 1);
    void reset();
private:
    uint64_t myCurrentTimestamp = 0;
};

template<typename Duration = std::chrono::nanoseconds, typename Func>
auto timeOperation(Func&& func) -> typename Duration::rep {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<Duration>(end - start).count();
}
}
}

#endif
