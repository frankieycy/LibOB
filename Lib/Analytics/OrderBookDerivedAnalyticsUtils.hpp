#ifndef ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Analytics/OrderBookObservables.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

struct BurnInPolicy {
    enum class Type {
        NONE,
        EVENT_COUNT,
        TRADE_COUNT,
        VOLUME,
        AUTO_CONVERGENCE
    };

    Type type = Type::NONE;

    uint64_t minEvents = 0;
    uint64_t minTrades = 0;
    double minVolume = 0.0;

    // auto-detection
    size_t windowSize = 10'000;
    size_t stableWindows = 5;
    double tolerance = 1e-3;
};

struct OrderBookTraces {
    Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> orderBookStatisticsCollector;
    Statistics::TimeSeriesCollector<Exchange::OrderProcessingReport> orderProcessingReportsCollector;
};

struct OrderDepthProfileConfig {
    OrderDepthProfileStats::DepthNormalization normalization = OrderDepthProfileStats::DepthNormalization::UNNORMALIZED;
    OrderDepthProfileStats::PriceSpaceDefinition priceSpace = OrderDepthProfileStats::PriceSpaceDefinition::DIFF_TO_OWN_BEST;
    size_t maxTicks = 10; // size of the depth profile in price ticks, chosen to be the same as MatchingEngineMonitor::myOrderBookNumLevels default
    double minPriceTick = 0.01;
    bool countMissingLevels = true;
};

struct OrderFlowMemoryStatsConfig {
    std::vector<size_t> lags = std::vector<size_t>(OrderFlowMemoryStats::DefaultLags.begin(), OrderFlowMemoryStats::DefaultLags.end());
};

struct PriceReturnScalingStatsConfig {
    PriceReturnScalingStats::PriceType priceType = PriceReturnScalingStats::PriceType::MID;
    bool logReturns = true;
};

struct MonitorOutputsAnalyzerConfig {
    bool debugMode = false;
};

struct OrderBookDerivedStatsConfig {
    OrderDepthProfileConfig orderDepthProfileConfig = OrderDepthProfileConfig();
    OrderFlowMemoryStatsConfig orderFlowMemoryStatsConfig = OrderFlowMemoryStatsConfig();
    PriceReturnScalingStatsConfig priceReturnScalingStatsConfig = PriceReturnScalingStatsConfig();
};
}

#endif
