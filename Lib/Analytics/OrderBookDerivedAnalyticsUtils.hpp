#ifndef ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Analytics/OrderBookObservables.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

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

struct PriceReturnScalingStatsConfig {
    PriceReturnScalingStats::PriceType priceType = PriceReturnScalingStats::PriceType::MID;
    bool logReturns = true;
};
}

#endif
