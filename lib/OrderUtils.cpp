#ifndef ORDER_UTILS_CPP
#define ORDER_UTILS_CPP
#include "Utils.hpp"
#include "OrderUtils.hpp"

namespace Market {
std::ostream& operator<<(std::ostream& out, const Side& side) {
    switch (side) {
        case Side::BUY:  out << "Buy";  break;
        case Side::SELL: out << "Sell"; break;
        default:         out << "Null"; break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const OrderType& orderType) {
    switch (orderType) {
        case OrderType::LIMIT:  out << "Limit";  break;
        case OrderType::MARKET: out << "Market"; break;
        default:                out << "Null";   break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const OrderState& orderState) {
    switch (orderState) {
        case OrderState::ACTIVE:         out << "Active";          break;
        case OrderState::FILLED:         out << "Filled";          break;
        case OrderState::PARTIAL_FILLED: out << "PartiallyFilled"; break;
        case OrderState::CANCELLED:      out << "Cancelled";       break;
        case OrderState::INVALID:        out << "Invalid";         break;
        default:                         out << "Null";            break;
    }
    return out;
}

const uint64_t OrderIdHandler::generateId() {
    ++myCurrentId;
    myIdLog.push_back(myCurrentId);
    return myCurrentId;
}

void OrderIdHandler::reset() {
    myCurrentId = 0;
    myIdLog.clear();
}

void OrderTimestampHandler::tick(const uint64_t elapsedTimeUnit) {
    myCurrentTimestamp += elapsedTimeUnit;
}

void OrderTimestampHandler::reset() {
    myCurrentTimestamp = 0;
}
}

#endif
