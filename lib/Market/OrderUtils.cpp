#ifndef ORDER_UTILS_CPP
#define ORDER_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"

namespace Market {
std::string to_string(const Side& side) {
    switch (side) {
        case Side::BUY:  return "Buy";
        case Side::SELL: return "Sell";
        default:         return "Null";
    }
}

std::string to_string(const OrderType& orderType) {
    switch (orderType) {
        case OrderType::LIMIT:  return "Limit";
        case OrderType::MARKET: return "Market";
        default:                return "Null";
    }
}

std::string to_string(const OrderState& orderState) {
    switch (orderState) {
        case OrderState::ACTIVE:         return "Active";
        case OrderState::FILLED:         return "Filled";
        case OrderState::PARTIAL_FILLED: return "PartialFilled";
        case OrderState::CANCELLED:      return "Cancelled";
        case OrderState::INVALID:        return "Invalid";
        default:                         return "Null";
    }
}

std::string to_string(const OrderEventType& orderEventType) {
    switch (orderEventType) {
        case OrderEventType::SUBMIT:          return "Submit";
        case OrderEventType::FILL:            return "Fill";
        case OrderEventType::CANCEL:          return "Cancel";
        case OrderEventType::MODIFY_PRICE:    return "ModifyPrice";
        case OrderEventType::MODIFY_QUANTITY: return "ModifyQuantity";
        default:                              return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const Side& side) { return out << to_string(side); }
std::ostream& operator<<(std::ostream& out, const OrderType& orderType) { return out << to_string(orderType); }
std::ostream& operator<<(std::ostream& out, const OrderState& orderState) { return out << to_string(orderState); }
std::ostream& operator<<(std::ostream& out, const OrderEventType& orderEventType) { return out << to_string(orderEventType); }
}

#endif
