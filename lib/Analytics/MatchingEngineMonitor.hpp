#ifndef MATCHING_ENGINE_MONITOR_HPP
#define MATCHING_ENGINE_MONITOR_HPP
#include "Utils/Utils.hpp"
#include "Market/Trade.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Parser/LobsterDataParser.hpp"

namespace Analytics {
using namespace Utils;
using namespace Parser;

class MatchingEngineMonitor {
public:
    enum class OrderBookStatisticsTimestampStrategy {
        TOP_OF_BOOK_TICK,  // default: log a new statistics entry per each top-of-book tick (order submit/cancel/modify events away from top-of-book are muted)
        EACH_ORDER_EVENT,  // log a new statistics entry per each order event (expensive!)
        EACH_MARKET_ORDER, // log a new statistics entry per each market order event
        EACH_TRADE,        // log a new statistics entry per each trade event (that almost always happens at top-of-book)
    };

    /* Order book top-level snapshot fetched directly from the matching engine */
    struct OrderBookTopLevelsSnapshot {
        size_t numLevels = 0;
        bool isFullBook = false;
        std::shared_ptr<const Market::TradeBase> lastTrade = nullptr;
        std::vector<Exchange::PriceLevel> bidBookTopPrices;
        std::vector<Exchange::PriceLevel> askBookTopPrices;
        std::vector<uint32_t> bidBookTopSizes;
        std::vector<uint32_t> askBookTopSizes;

        OrderBookTopLevelsSnapshot(const size_t numLevels = 0, const bool isFullBook = false) : numLevels(numLevels), isFullBook(isFullBook) {}
        // numLevels and isFullBook (configs) are first set before fetching data from matching engine
        void constructFrom(const std::shared_ptr<const Exchange::IMatchingEngine>& matchingEngine);
        void clear();
        std::string getAsJson() const;
        std::string getAsCsv() const;
        std::string getAsTable() const;
    };

    /* Order book statistics aggregated over all timestamps so far */
    struct OrderBookAggregateStatistics {
        uint64_t timestampFrom = 0;
        uint64_t timestampTo = 0;
        size_t aggNumNewLimitOrders = 0;
        size_t aggNumNewMarketOrders = 0;
        size_t aggNumCancelOrders = 0;
        size_t aggNumModifyPriceOrders = 0;
        size_t aggNumModifyQuantityOrders = 0;
        size_t aggNumTrades = 0;
        uint32_t aggTradeVolume = 0;
        double aggTradeNotional = 0.0;
        std::string getAsJson() const;
        std::string getAsCsv() const;
        std::string getAsTable() const;
    };

    /* Order book statistics between the from-time exclusive to the to-time inclusive derived from OrderBookTopLevelsSnapshot */
    struct OrderBookStatisticsByTimestamp {
        uint64_t timestampFrom = 0;
        uint64_t timestampTo = 0;
        size_t cumNumNewLimitOrders = 0;
        size_t cumNumNewMarketOrders = 0;
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
            const std::shared_ptr<const Exchange::IMatchingEngine>& matchingEngine,
            const OrderBookAggregateStatistics& orderBookAggregateStatistics,
            const OrderBookAggregateStatistics& orderBookAggregateStatisticsCache);
        void clear();
        std::string getAsJson() const;
        std::string getAsCsv() const;
        std::string getAsTable() const;
    };

    /* Processing latency measured over an order event */
    struct OrderEventProcessingLatency {
        uint64_t timestamp;
        uint64_t eventId;
        Market::OrderEventType eventType = Market::OrderEventType::NULL_ORDER_EVENT_TYPE;
        unsigned long long latency = 0; // chronos::microseconds::rep
        std::shared_ptr<const Market::OrderEventBase> event = nullptr;
        std::string getAsJson() const;
        std::string getAsCsv() const;
        std::string getAsTable() const;
    };

    MatchingEngineMonitor(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~MatchingEngineMonitor() = default;

    std::shared_ptr<Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    bool isDebugMode() const { return myDebugMode; }
    bool isMonitoringEnabled() const { return myMonitoringEnabled; }
    bool isFetchFullOrderBook() const { return myFetchFullOrderBook; }
    size_t getOrderBookNumLevels() const { return myOrderBookNumLevels; }
    size_t getTimeSeriesCollectorMaxSize() const { return myTimeSeriesCollectorMaxSize; }
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
    }
    void setOrderBookStatisticsTimestampStrategy(const OrderBookStatisticsTimestampStrategy strategy) { myOrderBookStatisticsTimestampStrategy = strategy; }

    virtual void init();
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
    OrderBookAggregateStatistics myOrderBookAggregateStatistics;
    OrderBookAggregateStatistics myOrderBookAggregateStatisticsCache; // caches the last statistics entry
    Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> myOrderBookStatisticsCollector;
    Statistics::TimeSeriesCollector<OrderEventProcessingLatency> myOrderEventProcessingLatenciesCollector;
    Statistics::TimeSeriesCollector<Exchange::OrderProcessingReport> myOrderProcessingReportsCollector;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> myOrderProcessingCallback;
    Exchange::CallbackSharedPtr<Exchange::OrderProcessingReport> mySharedOrderProcessingCallback; // constructed once in init()
    OrderBookStatisticsTimestampStrategy myOrderBookStatisticsTimestampStrategy = OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK;
};

std::string to_string(const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookTopLevelsSnapshot& snapshot);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookAggregateStatistics& stats);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsByTimestamp& stats);
std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderEventProcessingLatency& latency);
}

#endif
