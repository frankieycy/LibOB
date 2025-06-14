#ifndef MATCHING_ENGINE_MONITOR_HPP
#define MATCHING_ENGINE_MONITOR_HPP
#include "Utils/Utils.hpp"
#include "Market/Trade.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Analytics {
using namespace Utils;

class MatchingEngineMonitor {
public:
    /* Order book top-level snapshot fetched directly from the matching engine */
    struct OrderBookTopLevelsSnapshot {
        size_t numLevels = 0;
        std::shared_ptr<const Market::TradeBase> lastTrade = nullptr;
        Exchange::DescOrderBookSize bidBookTopLevels;
        Exchange::AscOrderBookSize askBookTopLevels;
    };

    /* Order book statistics derived from OrderBookTopLevelsSnapshot */
    struct OrderBookStatisticsByTimestamp {
        uint64_t timestamp;
        size_t cumNumNewOrders = 0;
        size_t cumNumCancelOrders = 0;
        size_t cumNumModifyPriceOrders = 0;
        size_t cumNumModifyQuantityOrders = 0;
        size_t cumNumTrades = 0;
        uint32_t cumTradeVolume = 0;
        double cumTradeNotional = 0.0;
        double bestBidPrice = Utils::Consts::NAN_DOUBLE;
        double bestAskPrice = Utils::Consts::NAN_DOUBLE;
        double midPrice = Utils::Consts::NAN_DOUBLE;
        double microPrice = Utils::Consts::NAN_DOUBLE;
        double spread = Utils::Consts::NAN_DOUBLE;
        double halfSpread = Utils::Consts::NAN_DOUBLE;
        double orderImbalance = Utils::Consts::NAN_DOUBLE;
        uint32_t bestBidSize = 0;
        uint32_t bestAskSize = 0;
        double lastTradePrice = Utils::Consts::NAN_DOUBLE;
        uint32_t lastTradeQuantity = 0.0;
        OrderBookTopLevelsSnapshot topLevelsSnapshot;
    };

    /* Order book statistics aggregated over all timestamps so far */
    struct OrderBookAggregateStatistics {
        size_t aggNumNewOrders = 0;
        size_t aggNumCancelOrders = 0;
        size_t aggNumModifyPriceOrders = 0;
        size_t aggNumModifyQuantityOrders = 0;
        size_t aggNumTrades = 0;
        uint32_t aggTradeVolume = 0;
        double aggTradeNotional = 0.0;
    };

    /* Processing latency measured over an order event */
    struct OrderEventProcessingLatency {
        uint64_t timestamp;
        uint64_t eventId;
        Market::OrderEventType eventType = Market::OrderEventType::NULL_ORDER_EVENT_TYPE;
        long long latency = 0; // chronos::microseconds::rep
        std::shared_ptr<const Market::OrderEventBase> event = nullptr;
    };

    MatchingEngineMonitor(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~MatchingEngineMonitor() = default;
    void startMonitoring() { myMonitoringEnabled = true; }
    void stopMonitoring() { myMonitoringEnabled = false; }

private:
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger;
    bool myDebugMode = false;
    bool myMonitoringEnabled = false;
    OrderBookAggregateStatistics myOrderBookAggregateStatistics;
    std::vector<std::shared_ptr<const OrderBookStatisticsByTimestamp>> myOrderBookStatistics;
    std::vector<std::shared_ptr<const OrderEventProcessingLatency>> myOrderEventProcessingLatencies;
};
}

#endif
