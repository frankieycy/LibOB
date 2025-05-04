#ifndef MATCHING_ENGINE_UTILS_HPP
#define MATCHING_ENGINE_UTILS_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"

namespace Exchange {
enum class OrderMatchingStrategy { FIFO, PRO_RATA, ICEBERG_SUPPORT, NULL_ORDER_MATCHING_STRATEGY };

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy);
}

#endif
