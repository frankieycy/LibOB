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
}

#endif
