#ifndef MATCHING_ENGINE_UTILS_CPP
#define MATCHING_ENGINE_UTILS_CPP
#include "Utils.hpp"
#include "MatchingEngineUtils.hpp"

namespace Exchange {
std::string to_string(const OrderMatchingStrategy& orderMatchingStrategy) {
    switch (orderMatchingStrategy) {
        case OrderMatchingStrategy::FIFO:            return "FIFO";
        case OrderMatchingStrategy::PRO_RATA:        return "ProRata";
        case OrderMatchingStrategy::ICEBERG_SUPPORT: return "IcebergSupport";
        default:                                     return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy) { return out << to_string(orderMatchingStrategy); }

OrderBookDisplayConfig::OrderBookDisplayConfig(const bool debugMode) :
    myDebugMode(debugMode) {
    if (myDebugMode) {
        myOrderBookLevels = 20;
        myMarketQueueLevels = 20;
        myTradeLogLevels = 20;
        myRemovedLimitOrderLogLevels = 20;
        myAggregateOrderBook = false;
        myShowOrderBook = true;
        myShowMarketQueue = true;
        myShowTradeLog = true;
        myShowRemovedLimitOrderLog = true;
        myShowOrderLookup = true;
    }
}
}

#endif
