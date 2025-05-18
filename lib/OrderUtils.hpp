#ifndef ORDER_UTILS_HPP
#define ORDER_UTILS_HPP
#include "Utils.hpp"

namespace Market {
enum class Side           { BUY, SELL, NULL_SIDE };
enum class OrderType      { LIMIT, MARKET, NULL_ORDER_TYPE };
enum class OrderState     { ACTIVE, FILLED, PARTIAL_FILLED, CANCELLED, INVALID, NULL_ORDER_STATE }; // order filling goes from ACTIVE to PARTIAL_FILLED to FILLED
enum class OrderEventType { SUBMIT, FILL, CANCEL, MODIFY_PRICE, MODIFY_QUANTITY, NULL_ORDER_EVENT_TYPE };

std::string to_string(const Side& side);
std::string to_string(const OrderType& orderType);
std::string to_string(const OrderState& orderState);
std::string to_string(const OrderEventType& orderEventType);

std::ostream& operator<<(std::ostream& out, const Side& side);
std::ostream& operator<<(std::ostream& out, const OrderType& orderType);
std::ostream& operator<<(std::ostream& out, const OrderState& orderState);
std::ostream& operator<<(std::ostream& out, const OrderEventType& orderEventType);
}

#endif
