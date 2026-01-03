#ifndef ORDER_UTILS_CPP
#define ORDER_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"

namespace Market {
std::string toString(const Side& side) {
    switch (side) {
        case Side::BUY:  return "Buy";
        case Side::SELL: return "Sell";
        default:         return "None";
    }
}

std::string toString(const OrderType& orderType) {
    switch (orderType) {
        case OrderType::LIMIT:  return "Limit";
        case OrderType::MARKET: return "Market";
        default:                return "None";
    }
}

std::string toString(const OrderState& orderState) {
    switch (orderState) {
        case OrderState::ACTIVE:         return "Active";
        case OrderState::FILLED:         return "Filled";
        case OrderState::PARTIAL_FILLED: return "PartialFilled";
        case OrderState::CANCELLED:      return "Cancelled";
        case OrderState::INVALID:        return "Invalid";
        default:                         return "None";
    }
}

std::string toString(const OrderEventType& orderEventType) {
    switch (orderEventType) {
        case OrderEventType::SUBMIT:          return "Submit";
        case OrderEventType::FILL:            return "Fill";
        case OrderEventType::CANCEL:          return "Cancel";
        case OrderEventType::PARTIAL_CANCEL:  return "PartialCancel";
        case OrderEventType::CANCEL_REPLACE:  return "CancelReplace";
        case OrderEventType::MODIFY_PRICE:    return "ModifyPrice";
        case OrderEventType::MODIFY_QUANTITY: return "ModifyQuantity";
        default:                              return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const Side& side) { return out << toString(side); }
std::ostream& operator<<(std::ostream& out, const OrderType& orderType) { return out << toString(orderType); }
std::ostream& operator<<(std::ostream& out, const OrderState& orderState) { return out << toString(orderState); }
std::ostream& operator<<(std::ostream& out, const OrderEventType& orderEventType) { return out << toString(orderEventType); }
}

#endif
