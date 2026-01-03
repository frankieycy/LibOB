#ifndef ORDER_BOOK_OBSERVABLES_HPP
#define ORDER_BOOK_OBSERVABLES_HPP
#include "Utils/Utils.hpp"
#include "Market/Trade.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Analytics {
using namespace Utils;

/* Order book top-level snapshot fetched directly from the matching engine. The class vector members are in ascending order of book levels,
    e.g. bidBookTopPrices = {99.0, 98.0, ...} and askBookTopPrices = {101.0, 102.0, ...}. */
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

/* Processing latency in real-world wall-clock time (nanoseconds) measured over an order event */
struct OrderEventProcessingLatency {
    uint64_t timestamp;
    uint64_t eventId;
    std::chrono::nanoseconds::rep latency = 0;
    Market::OrderEventType eventType = Market::OrderEventType::NULL_ORDER_EVENT_TYPE;
    std::shared_ptr<const Market::OrderEventBase> event = nullptr;

    OrderEventProcessingLatency(const std::shared_ptr<const Exchange::OrderEventLatency>& latency) :
        timestamp(latency->second->getTimestamp()),
        eventId(latency->second->getEventId()),
        latency(latency->first),
        eventType(latency->second->getEventType()),
        event(latency->second) {}
    std::string getAsJson() const;
    std::string getAsCsv() const;
    std::string getAsTable() const;
};

std::ostream& operator<<(std::ostream& out, const OrderBookTopLevelsSnapshot& snapshot);
std::ostream& operator<<(std::ostream& out, const OrderBookAggregateStatistics& stats);
std::ostream& operator<<(std::ostream& out, const OrderBookStatisticsByTimestamp& stats);
std::ostream& operator<<(std::ostream& out, const OrderEventProcessingLatency& latency);
}

#endif
