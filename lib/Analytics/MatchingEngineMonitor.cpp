#ifndef MATCHING_ENGINE_MONITOR_CPP
#define MATCHING_ENGINE_MONITOR_CPP
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

namespace Analytics {
using namespace Utils;

MatchingEngineMonitor::MatchingEngineMonitor(const std::shared_ptr<Exchange::MatchingEngineBase>& matchingEngine) :
    myMatchingEngine(matchingEngine), myDebugMode(matchingEngine->isDebugMode()), myMonitoringEnabled(false) {
    if (!matchingEngine)
        Error::LIB_THROW("[MatchingEngineMonitor] Matching engine is null.");
    init();
    myMatchingEngine = matchingEngine;
    // add the callback to the matching engine once, and manage its lifetime internally via start/stopMonitoring()
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
}

void MatchingEngineMonitor::init() {
    myOrderBookStatisticsCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderEventProcessingLatenciesCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    mySharedOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderExecutionReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumTrades;
    myOrderBookAggregateStatistics.aggTradeVolume += report.filledQuantity;
    myOrderBookAggregateStatistics.aggTradeNotional += report.filledPrice * report.filledQuantity;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewOrders;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewOrders;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
    ++myOrderBookAggregateStatistics.aggNumNewOrders;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyPriceOrders;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report) {
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyQuantityOrders;
}
}

#endif
