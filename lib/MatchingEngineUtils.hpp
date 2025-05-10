#ifndef MATCHING_ENGINE_UTILS_HPP
#define MATCHING_ENGINE_UTILS_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"

namespace Exchange {
enum class OrderMatchingStrategy { FIFO, PRO_RATA, ICEBERG_SUPPORT, NULL_ORDER_MATCHING_STRATEGY };

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy);

class OrderBookDisplayConfig {
public:
    OrderBookDisplayConfig() = default;
    OrderBookDisplayConfig(const bool debugMode);
    const uint16_t getOrderBookLevels() const { return myOrderBookLevels; }
    const uint16_t getMarketQueueLevels() const { return myMarketQueueLevels; }
    const uint16_t getTradeLogLevels() const { return myTradeLogLevels; }
    const uint16_t getRemovedLimitOrderLogLevels() const { return myRemovedLimitOrderLogLevels; }
    const bool isAggregateOrderBook() const { return myAggregateOrderBook; }
    const bool isShowOrderBook() const { return myShowOrderBook; }
    const bool isShowMarketQueue() const { return myShowMarketQueue; }
    const bool isShowTradeLog() const { return myShowTradeLog; }
    const bool isShowRemovedLimitOrderLog() const { return myShowRemovedLimitOrderLog; }
    const bool isShowOrderLookup() const { return myShowOrderLookup; }
    const bool isDebugMode() const { return myDebugMode; }
    void setOrderBookLevels(const uint16_t orderBookLevels) { myOrderBookLevels = orderBookLevels; }
    void setMarketQueueLevels(const uint16_t marketQueueLevels) { myMarketQueueLevels = marketQueueLevels; }
    void setTradeLogLevels(const uint16_t tradeLogLevels) { myTradeLogLevels = tradeLogLevels; }
    void setRemovedLimitOrderLogLevels(const uint16_t removedLimitOrderLogLevels) { myRemovedLimitOrderLogLevels = removedLimitOrderLogLevels; }
    void setAggregateOrderBook(const bool aggregateOrderBook) { myAggregateOrderBook = aggregateOrderBook; }
    void setShowOrderBook(const bool showOrderBook) { myShowOrderBook = showOrderBook; }
    void setShowMarketQueue(const bool showMarketQueue) { myShowMarketQueue = showMarketQueue; }
    void setShowTradeLog(const bool showTradeLog) { myShowTradeLog = showTradeLog; }
    void setShowRemovedLimitOrderLog(const bool showRemovedLimitOrderLog) { myShowRemovedLimitOrderLog = showRemovedLimitOrderLog; }
    void setShowOrderLookup(const bool showOrderLookup) { myShowOrderLookup = showOrderLookup; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; } // TODO: default values of config parameters
private:
    uint16_t myOrderBookLevels = 5;
    uint16_t myMarketQueueLevels = 10;
    uint16_t myTradeLogLevels = 10;
    uint16_t myRemovedLimitOrderLogLevels = 10;
    bool myAggregateOrderBook = true;
    bool myShowOrderBook = true;
    bool myShowMarketQueue = true;
    bool myShowTradeLog = true;
    bool myShowRemovedLimitOrderLog = true;
    bool myShowOrderLookup = true;
    bool myDebugMode = false;
};
}

#endif
