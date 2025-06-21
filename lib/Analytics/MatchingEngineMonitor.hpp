#ifndef MATCHING_ENGINE_MONITOR_HPP
#define MATCHING_ENGINE_MONITOR_HPP
#include "Utils/Utils.hpp"
#include "Market/Trade.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Analytics {
using namespace Utils;

class MatchingEngineMonitor {
public:
    enum class OrderBookStatisticsTimestampStrategy {
        EACH_ORDER_EVENT,  // log a new statistics entry per each order event (expensive!)
        EACH_MARKET_ORDER, // log a new statistics entry per each market order event
        EACH_TRADE,        // log a new statistics entry per each trade event (that almost always happens at top-of-book)
        TOP_OF_BOOK_TICK,  // log a new statistics entry per each top-of-book tick (order submit/cancel/modify events away from top-of-book are muted)
    };

    /* Order book top-level snapshot fetched directly from the matching engine */
    struct OrderBookTopLevelsSnapshot {
        size_t numLevels = 0;
        bool isFullBook = false;
        std::shared_ptr<const Market::TradeBase> lastTrade = nullptr;
        Exchange::DescOrderBookSize bidBookTopLevels;
        Exchange::AscOrderBookSize askBookTopLevels;

        OrderBookTopLevelsSnapshot(const size_t numLevels = 0, const bool isFullBook = false) : numLevels(numLevels), isFullBook(isFullBook) {}

        void constructFrom(const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine) {
            if (!matchingEngine)
                Error::LIB_THROW("[OrderBookTopLevelsSnapshot] Matching engine is null.");
            lastTrade = matchingEngine->getLastTrade();
            bidBookTopLevels = isFullBook ? matchingEngine->getBidBookSize() : matchingEngine->getBidBookSize(numLevels);
            askBookTopLevels = isFullBook ? matchingEngine->getAskBookSize() : matchingEngine->getAskBookSize(numLevels);
        }

        void clear() {
            numLevels = 0;
            isFullBook = false;
            lastTrade = nullptr;
            bidBookTopLevels.clear();
            askBookTopLevels.clear();
        }
    };

    /* Order book statistics aggregated over all timestamps so far */
    struct OrderBookAggregateStatistics {
        uint64_t timestampFrom = 0;
        uint64_t timestampTo = 0;
        size_t aggNumNewOrders = 0;
        size_t aggNumCancelOrders = 0;
        size_t aggNumModifyPriceOrders = 0;
        size_t aggNumModifyQuantityOrders = 0;
        size_t aggNumTrades = 0;
        uint32_t aggTradeVolume = 0;
        double aggTradeNotional = 0.0;
    };

    /* Order book statistics between the from-time exclusive to the to-time inclusive derived from OrderBookTopLevelsSnapshot */
    struct OrderBookStatisticsByTimestamp {
        uint64_t timestampFrom = 0;
        uint64_t timestampTo = 0;
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
        uint32_t lastTradeQuantity = 0;
        OrderBookTopLevelsSnapshot topLevelsSnapshot;

        OrderBookStatisticsByTimestamp(const size_t numLevels = 0, const bool isFullBook = false) : topLevelsSnapshot(numLevels, isFullBook) {}

        void constructFrom(
            const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine,
            const OrderBookAggregateStatistics& orderBookAggregateStatistics,
            const OrderBookAggregateStatistics& orderBookAggregateStatisticsCache) {
            if (!matchingEngine)
                Error::LIB_THROW("[OrderBookStatisticsByTimestamp] Matching engine is null.");
            timestampFrom = orderBookAggregateStatisticsCache.timestampTo; // from-time exclusive
            timestampTo = orderBookAggregateStatistics.timestampTo; // to-time inclusive
            cumNumNewOrders = orderBookAggregateStatistics.aggNumNewOrders - orderBookAggregateStatisticsCache.aggNumNewOrders;
            cumNumCancelOrders = orderBookAggregateStatistics.aggNumCancelOrders - orderBookAggregateStatisticsCache.aggNumCancelOrders;
            cumNumModifyPriceOrders = orderBookAggregateStatistics.aggNumModifyPriceOrders - orderBookAggregateStatisticsCache.aggNumModifyPriceOrders;
            cumNumModifyQuantityOrders = orderBookAggregateStatistics.aggNumModifyQuantityOrders - orderBookAggregateStatisticsCache.aggNumModifyQuantityOrders;
            cumNumTrades = orderBookAggregateStatistics.aggNumTrades - orderBookAggregateStatisticsCache.aggNumTrades;
            cumTradeVolume = orderBookAggregateStatistics.aggTradeVolume - orderBookAggregateStatisticsCache.aggTradeVolume;
            cumTradeNotional = orderBookAggregateStatistics.aggTradeNotional - orderBookAggregateStatisticsCache.aggTradeNotional;
            bestBidPrice = matchingEngine->getBestBidPrice();
            bestAskPrice = matchingEngine->getBestAskPrice();
            midPrice = matchingEngine->getMidPrice();
            microPrice = matchingEngine->getMicroPrice();
            spread = matchingEngine->getSpread();
            halfSpread = matchingEngine->getHalfSpread();
            orderImbalance = matchingEngine->getOrderImbalance();
            bestBidSize = matchingEngine->getBestBidSize();
            bestAskSize = matchingEngine->getBestAskSize();
            lastTradePrice = matchingEngine->getLastTradePrice();
            lastTradeQuantity = matchingEngine->getLastTradeSize();
            topLevelsSnapshot.constructFrom(matchingEngine);
        }
    };

    /* Processing latency measured over an order event */
    struct OrderEventProcessingLatency {
        uint64_t timestamp;
        uint64_t eventId;
        Market::OrderEventType eventType = Market::OrderEventType::NULL_ORDER_EVENT_TYPE;
        unsigned long long latency = 0; // chronos::microseconds::rep
        std::shared_ptr<const Market::OrderEventBase> event = nullptr;
    };

    MatchingEngineMonitor(const std::shared_ptr<Exchange::MatchingEngineBase>& matchingEngine);
    virtual ~MatchingEngineMonitor() = default;

    std::shared_ptr<Exchange::MatchingEngineBase> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    bool isDebugMode() const { return myDebugMode; }
    bool isMonitoringEnabled() const { return myMonitoringEnabled; }
    size_t getOrderBookNumLevels() const { return myOrderBookNumLevels; }
    size_t getTimeSeriesCollectorMaxSize() const { return myTimeSeriesCollectorMaxSize; }
    OrderBookStatisticsTimestampStrategy getOrderBookStatisticsTimestampStrategy() const { return myOrderBookStatisticsTimestampStrategy; }
    OrderBookAggregateStatistics getOrderBookAggregateStatistics() { return myOrderBookAggregateStatistics; }
    const Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp>& getOrderBookStatistics() const { return myOrderBookStatisticsCollector; }
    const Statistics::TimeSeriesCollector<OrderEventProcessingLatency>& getOrderEventProcessingLatencies() const { return myOrderEventProcessingLatenciesCollector; }

    void setMatchingEngine(const std::shared_ptr<Exchange::MatchingEngineBase>& matchingEngine) { myMatchingEngine = matchingEngine; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }
    void setOrderBookNumLevels(const size_t numLevels) { myOrderBookNumLevels = numLevels; }
    void setTimeSeriesCollectorMaxSize(const size_t maxSize) {
        myTimeSeriesCollectorMaxSize = maxSize;
        myOrderBookStatisticsCollector.setMaxHistory(maxSize);
        myOrderEventProcessingLatenciesCollector.setMaxHistory(maxSize);
    }
    void setOrderBookStatisticsTimestampStrategy(const OrderBookStatisticsTimestampStrategy strategy) { myOrderBookStatisticsTimestampStrategy = strategy; }

    virtual void init();
    virtual void startMonitoring() {
        myOrderProcessingCallback = mySharedOrderProcessingCallback;
        myMonitoringEnabled = true;
    }
    virtual void stopMonitoring() {
        myOrderProcessingCallback = nullptr;
        myMonitoringEnabled = false;
    }

    // communicates with matching engine to keep order book stats in sync
    virtual void onOrderProcessingReport(const Exchange::OrderExecutionReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report);

private:
    std::shared_ptr<Exchange::MatchingEngineBase> myMatchingEngine;
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    bool myDebugMode = false;
    bool myMonitoringEnabled = false;
    size_t myOrderBookNumLevels = 10;
    size_t myTimeSeriesCollectorMaxSize = 10000;
    OrderBookAggregateStatistics myOrderBookAggregateStatistics;
    OrderBookAggregateStatistics myOrderBookAggregateStatisticsCache; // caches the last statistics entry
    Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> myOrderBookStatisticsCollector;
    Statistics::TimeSeriesCollector<OrderEventProcessingLatency> myOrderEventProcessingLatenciesCollector;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> myOrderProcessingCallback;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> mySharedOrderProcessingCallback; // constructed once in init()
    OrderBookStatisticsTimestampStrategy myOrderBookStatisticsTimestampStrategy = OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK;
};
}

#endif
