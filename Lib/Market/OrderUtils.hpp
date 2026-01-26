#ifndef ORDER_UTILS_HPP
#define ORDER_UTILS_HPP
#include "Utils/Utils.hpp"

namespace Market {
enum class Side           { BUY, SELL, NONE };
enum class OrderType      { LIMIT, MARKET, NONE };
enum class OrderState     { ACTIVE, FILLED, PARTIAL_FILLED, CANCELLED, INVALID, NONE }; // order filling goes from ACTIVE to PARTIAL_FILLED to FILLED
enum class OrderEventType { SUBMIT, FILL, CANCEL, PARTIAL_CANCEL, CANCEL_REPLACE, MODIFY_PRICE, MODIFY_QUANTITY, BROKEN_TRADE, NONE };
}

template<>
struct Utils::EnumStrings<Market::Side> {
    inline static constexpr std::array<const char*, 3> names = { "Buy", "Sell", "None" };
};

template<>
struct Utils::EnumStrings<Market::OrderType> {
    inline static constexpr std::array<const char*, 3> names = { "Limit", "Market", "None" };
};

template<>
struct Utils::EnumStrings<Market::OrderState> {
    inline static constexpr std::array<const char*, 6> names = { "Active", "Filled", "PartialFilled", "Cancelled", "Invalid", "None" };
};

template<>
struct Utils::EnumStrings<Market::OrderEventType> {
    inline static constexpr std::array<const char*, 9> names = { "Submit", "Fill", "Cancel", "PartialCancel", "CancelReplace", "ModifyPrice", "ModifyQuantity", "BrokenTrade", "None" };
};

#endif
