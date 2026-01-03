#ifndef MATCHING_ENGINE_MONITOR_HPP
#define MATCHING_ENGINE_MONITOR_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Parser/LobsterDataParser.hpp"
#include "Analytics/OrderBookObservables.hpp"

namespace Analytics {
using namespace Utils;
using namespace Parser;

/* This monitor class is hefty as each tick detection logs a deep copy of the order book up to some level cutoff,
    with memory size linear in the number of tick detections */
class MatchingEngineMonitor {
public:
    enum class OrderBookStatisticsTimestampStrategy {
        TOP_OF_BOOK_TICK,  // default: log a new statistics entry per each top-of-book tick (order submit/cancel/modify events away from top-of-book are muted)
        EACH_ORDER_EVENT,  // log a new statistics entry per each order event (expensive!)
        EACH_MARKET_ORDER, // log a new statistics entry per each market order event
        EACH_TRADE,        // log a new statistics entry per each trade event (that almost always happens at top-of-book)
    };

    MatchingEngineMonitor() = delete; // only permits construction from matching engine
    MatchingEngineMonitor(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~MatchingEngineMonitor() = default;

    std::shared_ptr<Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    bool isDebugMode() const { return myDebugMode; }
    bool isMonitoringEnabled() const { return myMonitoringEnabled; }
    bool isFetchFullOrderBook() const { return myFetchFullOrderBook; }
    size_t getOrderBookNumLevels() const { return myOrderBookNumLevels; }
    size_t getTimeSeriesCollectorMaxSize() const { return myTimeSeriesCollectorMaxSize; }
    double getMinimumPriceTick() const { return myMinimumPriceTick; }
    OrderBookStatisticsTimestampStrategy getOrderBookStatisticsTimestampStrategy() const { return myOrderBookStatisticsTimestampStrategy; }
    OrderBookAggregateStatistics getOrderBookAggregateStatistics() { return myOrderBookAggregateStatistics; }
    const Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp>& getOrderBookStatistics() const { return myOrderBookStatisticsCollector; }
    const Statistics::TimeSeriesCollector<OrderEventProcessingLatency>& getOrderEventProcessingLatencies() const { return myOrderEventProcessingLatenciesCollector; }
    const OrderBookTopLevelsSnapshot& getLastOrderBookTopLevelsSnapshot() const;
    bool isPriceWithinTopOfBook(const Market::Side side, const double price, const std::optional<Market::OrderType>& type = std::nullopt) const;

    void setMatchingEngine(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) { myMatchingEngine = matchingEngine; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }
    void setFetchFullOrderBook(const bool fetchFullOrderBook) { myFetchFullOrderBook = fetchFullOrderBook; }
    void setOrderBookNumLevels(const size_t numLevels) { myOrderBookNumLevels = numLevels; }
    void setTimeSeriesCollectorMaxSize(const size_t maxSize) {
        myTimeSeriesCollectorMaxSize = maxSize;
        myOrderBookStatisticsCollector.setMaxHistory(maxSize);
        myOrderEventProcessingLatenciesCollector.setMaxHistory(maxSize);
        myOrderProcessingReportsCollector.setMaxHistory(maxSize);
    }
    void setMinimumPriceTick(const double minPriceTick) { myMinimumPriceTick = minPriceTick; }
    void setOrderBookStatisticsTimestampStrategy(const OrderBookStatisticsTimestampStrategy strategy) { myOrderBookStatisticsTimestampStrategy = strategy; }

    virtual void init();
    virtual void reset(const bool keepLastSnapshot = false);
    virtual void startMonitoring();
    virtual void stopMonitoring();
    virtual void updateStatistics(const Exchange::OrderProcessingReport& report);

    virtual void exportToLobsterDataParser(Parser::LobsterDataParser& parser) const;

    // communicates with matching engine to keep order book stats in sync
    virtual void onOrderProcessingReport(const Exchange::OrderExecutionReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderPlacementReport& report);
    virtual void onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderPartialCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report);

private:
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    std::shared_ptr<const Market::TradeBase> myLastTrade; // caches the last trade to uniquely count executions sent from both sides of the trade
    bool myDebugMode = false;
    bool myMonitoringEnabled = false;
    bool myFetchFullOrderBook = false;
    size_t myOrderBookNumLevels = 10; // used to detect top-of-book ticks
    size_t myTimeSeriesCollectorMaxSize = 1000000;
    double myMinimumPriceTick = 0.01; // for informational purpose in sync with order event manager which manages the min price tick
    OrderBookAggregateStatistics myOrderBookAggregateStatistics;
    OrderBookAggregateStatistics myOrderBookAggregateStatisticsCache; // caches the last statistics entry
    Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> myOrderBookStatisticsCollector;
    Statistics::TimeSeriesCollector<OrderEventProcessingLatency> myOrderEventProcessingLatenciesCollector;
    Statistics::TimeSeriesCollector<Exchange::OrderProcessingReport> myOrderProcessingReportsCollector;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> myOrderProcessingCallback;
    Exchange::CallbackSharedPtr<Exchange::OrderEventLatency> myOrderEventLatencyCallback;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> mySharedOrderProcessingCallback; // constructed once in init()
    OrderBookStatisticsTimestampStrategy myOrderBookStatisticsTimestampStrategy = OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK;
};

std::string to_string(const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy);
}

#endif
