#ifndef MATCHING_ENGINE_MONITOR_CPP
#define MATCHING_ENGINE_MONITOR_CPP
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

namespace Analytics {
using namespace Utils;

std::string to_string(const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) {
    switch (strategy) {
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK:  return "TopOfBookTick";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_ORDER_EVENT:  return "EachOrderEvent";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_MARKET_ORDER: return "EachMarketOrder";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_TRADE:        return "EachTrade";
        default:                                                                             return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) { return out << to_string(strategy); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookTopLevelsSnapshot& snapshot) { return out << snapshot.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookAggregateStatistics& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsByTimestamp& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderEventProcessingLatency& latency) { return out << latency.getAsJson(); }

void MatchingEngineMonitor::OrderBookTopLevelsSnapshot::constructFrom(const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderBookTopLevelsSnapshot] Matching engine is null.");
    lastTrade = matchingEngine->getLastTrade();
    bidBookTopLevelIterators = isFullBook ? matchingEngine->getBidBookSizeIterators() : matchingEngine->getBidBookSizeIterators(numLevels);
    askBookTopLevelIterators = isFullBook ? matchingEngine->getAskBookSizeIterators() : matchingEngine->getAskBookSizeIterators(numLevels);
}

void MatchingEngineMonitor::OrderBookTopLevelsSnapshot::clear() {
    numLevels = 0;
    isFullBook = false;
    lastTrade = nullptr;
    bidBookTopLevelIterators.clear();
    askBookTopLevelIterators.clear();
}

std::string MatchingEngineMonitor::OrderBookTopLevelsSnapshot::getAsJson() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderBookTopLevelsSnapshot::getAsCsv() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderBookAggregateStatistics::getAsJson() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderBookAggregateStatistics::getAsCsv() const {
    return ""; // TODO
}

void MatchingEngineMonitor::OrderBookStatisticsByTimestamp::constructFrom(
    const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine,
    const OrderBookAggregateStatistics& orderBookAggregateStatistics,
    const OrderBookAggregateStatistics& orderBookAggregateStatisticsCache) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderBookStatisticsByTimestamp] Matching engine is null.");
    timestampFrom = orderBookAggregateStatisticsCache.timestampTo; // from-time exclusive
    timestampTo = orderBookAggregateStatistics.timestampTo; // to-time inclusive
    cumNumNewLimitOrders = orderBookAggregateStatistics.aggNumNewLimitOrders - orderBookAggregateStatisticsCache.aggNumNewLimitOrders;
    cumNumNewMarketOrders = orderBookAggregateStatistics.aggNumNewMarketOrders - orderBookAggregateStatisticsCache.aggNumNewMarketOrders;
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

void MatchingEngineMonitor::OrderBookStatisticsByTimestamp::clear() {
    timestampFrom = 0;
    timestampTo = 0;
    cumNumNewLimitOrders = 0;
    cumNumNewMarketOrders = 0;
    cumNumCancelOrders = 0;
    cumNumModifyPriceOrders = 0;
    cumNumModifyQuantityOrders = 0;
    cumNumTrades = 0;
    cumTradeVolume = 0;
    cumTradeNotional = 0.0;
    bestBidPrice = Utils::Consts::NAN_DOUBLE;
    bestAskPrice = Utils::Consts::NAN_DOUBLE;
    midPrice = Utils::Consts::NAN_DOUBLE;
    microPrice = Utils::Consts::NAN_DOUBLE;
    spread = Utils::Consts::NAN_DOUBLE;
    halfSpread = Utils::Consts::NAN_DOUBLE;
    orderImbalance = Utils::Consts::NAN_DOUBLE;
    bestBidSize = 0;
    bestAskSize = 0;
    lastTradePrice = Utils::Consts::NAN_DOUBLE;
    lastTradeQuantity = 0;
    topLevelsSnapshot.clear();
}

std::string MatchingEngineMonitor::OrderBookStatisticsByTimestamp::getAsJson() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderBookStatisticsByTimestamp::getAsCsv() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderEventProcessingLatency::getAsJson() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderEventProcessingLatency::getAsCsv() const {
    return ""; // TODO
}

MatchingEngineMonitor::MatchingEngineMonitor(const std::shared_ptr<Exchange::MatchingEngineBase>& matchingEngine) :
    myMatchingEngine(matchingEngine), myDebugMode(matchingEngine->isDebugMode()), myMonitoringEnabled(true) {
    if (!matchingEngine)
        Error::LIB_THROW("[MatchingEngineMonitor] Matching engine is null.");
    init();
    myMatchingEngine = matchingEngine;
    // add the callback to the matching engine once, and manage its lifetime internally via start/stopMonitoring()
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
}

const MatchingEngineMonitor::OrderBookTopLevelsSnapshot& MatchingEngineMonitor::getLastOrderBookTopLevelsSnapshot() const {
    static const MatchingEngineMonitor::OrderBookTopLevelsSnapshot emptySnapshot;
    const auto& lastStats = myOrderBookStatisticsCollector.getLastSample();
    return lastStats ? lastStats->topLevelsSnapshot : emptySnapshot;
}

bool MatchingEngineMonitor::isPriceWithinTopOfBook(const Market::Side side, const double price, const std::optional<Market::OrderType>& type) const {
    if (type.has_value()) {
        if (*type == Market::OrderType::MARKET)
            return true;
    }
    const auto& topLevels = getLastOrderBookTopLevelsSnapshot();
    if (topLevels.isFullBook)
        return true;
    return (side == Market::Side::BUY && (topLevels.bidBookTopLevelIterators.empty() || price >= (*topLevels.bidBookTopLevelIterators.rbegin())->first)) ||
           (side == Market::Side::SELL && (topLevels.askBookTopLevelIterators.empty() || price <= (*topLevels.askBookTopLevelIterators.rbegin())->first));
}

void MatchingEngineMonitor::init() {
    myOrderBookStatisticsCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderEventProcessingLatenciesCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderProcessingCallback = mySharedOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
}

void MatchingEngineMonitor::startMonitoring() {
    myOrderProcessingCallback = mySharedOrderProcessingCallback;
    myMonitoringEnabled = true;
}

void MatchingEngineMonitor::stopMonitoring() {
    myOrderProcessingCallback = nullptr;
    myMonitoringEnabled = false;
}

void MatchingEngineMonitor::updateStatistics() {
    auto orderBookStats = std::make_shared<OrderBookStatisticsByTimestamp>(myOrderBookNumLevels, myFetchFullOrderBook);
    orderBookStats->constructFrom(myMatchingEngine, myOrderBookAggregateStatistics, myOrderBookAggregateStatisticsCache);
    myOrderBookStatisticsCollector.addSample(orderBookStats);
    myOrderBookAggregateStatisticsCache = myOrderBookAggregateStatistics;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderExecutionReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order execution report received: " << report;
    if (myLastTrade && report.tradeId == myLastTrade->getId())
        return; // avoid double-counting execution reports for the same trade (sent twice from both sides)
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumTrades;
    myOrderBookAggregateStatistics.aggTradeVolume += report.filledQuantity;
    myOrderBookAggregateStatistics.aggTradeNotional += report.filledPrice * report.filledQuantity;
    myLastTrade = myMatchingEngine->getLastTrade();
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.filledPrice))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order submit report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewLimitOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.order ? report.order->getPrice() : Consts::NAN_DOUBLE, Market::OrderType::LIMIT))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order submit report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewMarketOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.order ? report.order->getPrice() : Consts::NAN_DOUBLE, Market::OrderType::MARKET))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order cancel report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice.value_or(Consts::NAN_DOUBLE), report.orderType))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order cancel and replace report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
    ++myOrderBookAggregateStatistics.aggNumNewLimitOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.newPrice, report.orderType))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order modify price report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyPriceOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.modifiedPrice))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order modify quantity report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyQuantityOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice))
            return;
        updateStatistics();
    } else {
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    }
}
}

#endif
