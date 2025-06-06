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
    // TODO: hook into matching engine by setting a callback for order processing reports, but this clashes
    // with the callback set by OrderEventManagerBase - maybe make it into a vector of function pointers?
}
}

#endif
