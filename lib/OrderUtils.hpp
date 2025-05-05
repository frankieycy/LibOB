#ifndef ORDER_UTILS_HPP
#define ORDER_UTILS_HPP
#include "Utils.hpp"

namespace Market {
enum class Side           { BUY, SELL, NULL_SIDE };
enum class OrderType      { LIMIT, MARKET, NULL_ORDER_TYPE };
enum class OrderState     { ACTIVE, FILLED, PARTIAL_FILLED, CANCELLED, INVALID, NULL_ORDER_STATE };
enum class OrderEventType { SUBMIT, FILL, CANCEL, MODIFY_PRICE, MODIFY_QUANTITY, NULL_ORDER_EVENT_TYPE };

std::ostream& operator<<(std::ostream& out, const Side& side);
std::ostream& operator<<(std::ostream& out, const OrderType& orderType);
std::ostream& operator<<(std::ostream& out, const OrderState& orderState);
std::ostream& operator<<(std::ostream& out, const OrderEventType& orderEventType);

class OrderIdHandler {
public:
    OrderIdHandler() = default;
    const uint64_t getCurrentId() const { return myCurrentId; }
    const std::vector<uint64_t>& getIdLog() const { return myIdLog; }
    const uint64_t generateId();
    void reset();
private:
    uint64_t myCurrentId = 0;
    std::vector<uint64_t> myIdLog;
};

class OrderTimestampHandler {
public:
    OrderTimestampHandler() = default;
    const uint64_t getCurrentTimestamp() const { return myCurrentTimestamp; }
    void tick(const uint64_t elapsedTimeUnit = 1);
    void reset();
private:
    uint64_t myCurrentTimestamp = 0;
};
}

#endif
