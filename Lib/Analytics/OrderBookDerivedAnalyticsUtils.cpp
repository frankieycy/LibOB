#ifndef ORDER_BOOK_DERIVED_ANALYTICS_UTILS_CPP
#define ORDER_BOOK_DERIVED_ANALYTICS_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookDerivedAnalyticsUtils.hpp"

namespace Analytics {
using namespace Utils;

std::string toString(const MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode& accumulationMode) {
    switch (accumulationMode) {
        case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::ALL:             return "All";
        case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::EACH_TRADE:      return "EachTrade";
        case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::EACH_EVENT:      return "EachEvent";
        case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::LEVEL_ONE_TICK:  return "LevelOneTick";
        default:                                                                            return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode& accumulationMode) { return out << toString(accumulationMode); }
}

#endif
