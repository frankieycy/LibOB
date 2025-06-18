#ifndef MATCHING_ENGINE_MONITOR_CPP
#define MATCHING_ENGINE_MONITOR_CPP
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

namespace Analytics {
using namespace Utils;

MatchingEngineMonitor::MatchingEngineMonitor(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    myMatchingEngine(matchingEngine), myDebugMode(matchingEngine->isDebugMode()), myMonitoringEnabled(false) {
    if (!matchingEngine)
        Error::LIB_THROW("[MatchingEngineMonitor] Matching engine is null.");
    myMatchingEngine = matchingEngine;
    // add the callback to the matching engine once, and manage its lifetime internally via start/stopMonitoring()
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
    init();
}

void MatchingEngineMonitor::init() {
    mySharedOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderExecutionReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& /* report */) {
    return; // TODO
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& /* report */) {
    return; // TODO
}
}

#endif
