#ifndef ORDER_UTILS_HPP
#define ORDER_UTILS_HPP
#include "Utils/Utils.hpp"

namespace Market {
enum class Side           { BUY, SELL, NONE };
enum class OrderType      { LIMIT, MARKET, NONE };
enum class OrderState     { ACTIVE, FILLED, PARTIAL_FILLED, CANCELLED, INVALID, NONE }; // order filling goes from ACTIVE to PARTIAL_FILLED to FILLED
enum class OrderEventType { SUBMIT, FILL, CANCEL, PARTIAL_CANCEL, CANCEL_REPLACE, MODIFY_PRICE, MODIFY_QUANTITY, BROKEN_TRADE, NONE };

std::string toString(const Side& side);
std::string toString(const OrderType& orderType);
std::string toString(const OrderState& orderState);
std::string toString(const OrderEventType& orderEventType);

std::ostream& operator<<(std::ostream& out, const Side& side);
std::ostream& operator<<(std::ostream& out, const OrderType& orderType);
std::ostream& operator<<(std::ostream& out, const OrderState& orderState);
std::ostream& operator<<(std::ostream& out, const OrderEventType& orderEventType);
}

#endif
