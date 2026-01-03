#ifndef MATCHING_ENGINE_MONITOR_CPP
#define MATCHING_ENGINE_MONITOR_CPP
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/OrderBookObservables.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Parser/LobsterDataParser.hpp"

namespace Analytics {
using namespace Utils;

std::string toString(const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) {
    switch (strategy) {
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK:  return "TopOfBookTick";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_ORDER_EVENT:  return "EachOrderEvent";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_MARKET_ORDER: return "EachMarketOrder";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_TRADE:        return "EachTrade";
        default:                                                                             return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) { return out << toString(strategy); }

MatchingEngineMonitor::MatchingEngineMonitor(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    myMatchingEngine(matchingEngine), myDebugMode(matchingEngine->isDebugMode()), myMonitoringEnabled(true) {
    if (!matchingEngine)
        Error::LIB_THROW("[MatchingEngineMonitor] Matching engine is null.");
    init();
    myMatchingEngine = matchingEngine;
    // add the callback to the matching engine once, and manage its lifetime internally via start/stopMonitoring()
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
    matchingEngine->addOrderEventLatencyCallback(myOrderEventLatencyCallback);
}

const OrderBookTopLevelsSnapshot& MatchingEngineMonitor::getLastOrderBookTopLevelsSnapshot() const {
    static const OrderBookTopLevelsSnapshot emptySnapshot;
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
    return (side == Market::Side::BUY && (topLevels.bidBookTopPrices.empty() || topLevels.bidBookTopPrices.size() < myOrderBookNumLevels || price >= topLevels.bidBookTopPrices.back())) ||
           (side == Market::Side::SELL && (topLevels.askBookTopPrices.empty() || topLevels.askBookTopPrices.size() < myOrderBookNumLevels || price <= topLevels.askBookTopPrices.back()));
}

void MatchingEngineMonitor::init() {
    myOrderBookStatisticsCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderEventProcessingLatenciesCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderProcessingReportsCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderProcessingCallback = mySharedOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
    myOrderEventLatencyCallback = std::make_shared<Exchange::OrderEventLatencyCallback>(
        [this](const std::shared_ptr<const Exchange::OrderEventLatency>& latency) {
            myOrderEventProcessingLatenciesCollector.addSample(std::make_shared<OrderEventProcessingLatency>(latency));
        });
}

void MatchingEngineMonitor::reset(const bool keepLastSnapshot) {
    myOrderBookAggregateStatistics = OrderBookAggregateStatistics();
    myOrderBookAggregateStatisticsCache = OrderBookAggregateStatistics();
    myOrderEventProcessingLatenciesCollector.clear();
    if (keepLastSnapshot) {
        auto lastStats = myOrderBookStatisticsCollector.getLastSample();
        auto lastReport = myOrderProcessingReportsCollector.getLastSample();
        myOrderBookStatisticsCollector.clear();
        myOrderProcessingReportsCollector.clear();
        if (lastStats && lastReport) {
            myOrderBookStatisticsCollector.addSample(lastStats);
            myOrderProcessingReportsCollector.addSample(lastReport);
        }
    } else {
        myLastTrade = nullptr;
        myOrderBookStatisticsCollector.clear();
        myOrderProcessingReportsCollector.clear();
    }
}

void MatchingEngineMonitor::startMonitoring() {
    myOrderProcessingCallback = mySharedOrderProcessingCallback;
    myMonitoringEnabled = true;
}

void MatchingEngineMonitor::stopMonitoring() {
    myOrderProcessingCallback = nullptr;
    myMonitoringEnabled = false;
}

void MatchingEngineMonitor::updateStatistics(const Exchange::OrderProcessingReport& report) {
    auto orderBookStats = std::make_shared<OrderBookStatisticsByTimestamp>(myOrderBookNumLevels, myFetchFullOrderBook);
    orderBookStats->constructFrom(myMatchingEngine, myOrderBookAggregateStatistics, myOrderBookAggregateStatisticsCache);
    myOrderBookStatisticsCollector.addSample(orderBookStats);
    myOrderProcessingReportsCollector.addSample(report.clone());
    myOrderBookAggregateStatisticsCache = myOrderBookAggregateStatistics;
    if (myDebugMode) {
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Added new order book statistics snapshot:\n" << orderBookStats->getAsTable();
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Updated order book aggregate statistics:\n" << myOrderBookAggregateStatistics.getAsTable();
    }
}

void MatchingEngineMonitor::exportToLobsterDataParser(Parser::LobsterDataParser& parser) const {
    if (myOrderProcessingReportsCollector.size() != myOrderBookStatisticsCollector.size())
        Error::LIB_THROW("[MatchingEngineMonitor::exportToLobsterDataParser] Mismatched number of order processing reports (" +
            std::to_string(myOrderProcessingReportsCollector.size()) + ") and order book statistics (" +
            std::to_string(myOrderBookStatisticsCollector.size()) + ").");
    const auto& reports = myOrderProcessingReportsCollector.getSamples();
    const auto& stats = myOrderBookStatisticsCollector.getSamples();
    auto reportsIt = reports.begin();
    auto statsIt = stats.begin();
    for (; reportsIt != reports.end() && statsIt != stats.end(); ++reportsIt, ++statsIt) {
        const auto& topLevelsSnapshot = (*statsIt)->topLevelsSnapshot;
        const auto message = (*reportsIt)->makeLobsterMessage();
        std::vector<uint32_t> bidPricesInt((*statsIt)->topLevelsSnapshot.bidBookTopPrices.size());
        std::vector<uint32_t> askPricesInt((*statsIt)->topLevelsSnapshot.askBookTopPrices.size());
        std::transform(
            (*statsIt)->topLevelsSnapshot.bidBookTopPrices.begin(),
            (*statsIt)->topLevelsSnapshot.bidBookTopPrices.end(),
            bidPricesInt.begin(), [](double p){ return Maths::castDoublePriceAsInt<uint32_t>(p); });
        std::transform(
            (*statsIt)->topLevelsSnapshot.askBookTopPrices.begin(),
            (*statsIt)->topLevelsSnapshot.askBookTopPrices.end(),
            askPricesInt.begin(), [](double p){ return Maths::castDoublePriceAsInt<uint32_t>(p); });
        auto snapshot = std::make_shared<const Parser::LobsterDataParser::OrderBookSnapshot>(
            bidPricesInt, askPricesInt, topLevelsSnapshot.bidBookTopSizes, topLevelsSnapshot.askBookTopSizes);
        if (message->isValid()) {
            parser.addOrderBookMessageAndSnapshot(message, snapshot);
        } else if (message->toSplitIntoDeleteAndAdd()) {
            // split order modify and cancel/replace report into atomic delete and add messages
            const auto atomMessages = (*reportsIt)->decomposeIntoAtomicReports();
            if (atomMessages.size() == 2 &&
                atomMessages[0]->orderProcessingType == Exchange::OrderProcessingType::CANCEL &&
                atomMessages[1]->orderProcessingType == Exchange::OrderProcessingType::SUBMIT) {
                const auto messageDel = atomMessages[0]->makeLobsterMessage();
                const auto messageAdd = atomMessages[1]->makeLobsterMessage();
                parser.addOrderBookMessageAndSnapshot(messageDel, nullptr);
                parser.addOrderBookMessageAndSnapshot(messageAdd, snapshot);
            }
        }
    }
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
    if (!myLastTrade->getIsBuyLimitOrder() || !myLastTrade->getIsSellLimitOrder())
        ++myOrderBookAggregateStatistics.aggNumNewMarketOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.filledPrice))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report) {
    // note that order submit report does NOT trigger statistics update since it only indicates that the matching engine
    // received the order but the actual order placement is signaled by the order placement report
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Limit order submit report received: " << report;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderPlacementReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Limit order placement report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewLimitOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice, Market::OrderType::LIMIT))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report) {
    // similar to limit order submit report, no statistics update triggered here
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Market order submit report received: " << report;
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
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderPartialCancelReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order partial cancel report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyQuantityOrders; // partial cancel represented as a modify quantity order
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice.value_or(Consts::NAN_DOUBLE), report.orderType))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
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
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
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
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
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
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + toString(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}
}

#endif
