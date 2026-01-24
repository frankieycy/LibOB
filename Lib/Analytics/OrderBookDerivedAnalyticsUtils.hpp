#ifndef ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Analytics/OrderBookObservables.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

struct BurnInPolicy {
    enum class Type { EVENT_COUNT, TRADE_COUNT, VOLUME, AUTO_CONVERGENCE, NONE };
    Type type = Type::NONE;

    std::optional<uint32_t> minEvents;
    std::optional<uint32_t> minTrades;
    std::optional<double> minVolume;

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
    std::vector<uint64_t> horizons = std::vector<uint64_t>(PriceReturnScalingStats::DefaultHorizons.begin(), PriceReturnScalingStats::DefaultHorizons.end());
    PriceReturnScalingStats::PriceType priceType = PriceReturnScalingStats::PriceType::MID;
    bool logReturns = true;
};

struct SpreadStatsConfig {
    double minSpread = 0.0;
    double maxSpread = 1000.0;
    size_t numBins = 100000;
    Statistics::Histogram::Binning binning = Statistics::Histogram::Binning::UNIFORM;
    std::vector<size_t> lags = std::vector<size_t>(SpreadStats::DefaultLags.begin(), SpreadStats::DefaultLags.end());
};

struct EventTimeStatsConfig {
    double minEventTime = 0.0;
    double maxEventTime = 1000.0;
    size_t numBins = 1000;
    Statistics::Histogram::Binning binning = Statistics::Histogram::Binning::UNIFORM;
    EventTimeStats::PriceType priceType = EventTimeStats::PriceType::MID;
};

struct OrderLifetimeStatsConfig {
    OrderLifetimeStats::PriceSpaceDefinition priceSpace = OrderLifetimeStats::PriceSpaceDefinition::DIFF_TO_OWN_BEST;
    size_t maxTicks = 10; // size of the price buckets in price ticks of the histograms, chosen to be the same as MatchingEngineMonitor::myOrderBookNumLevels default
    double minPriceTick = 0.01;
    double minLifetime = 0.0;
    double maxLifetime = 1000.0;
    size_t numBins = 1000;
    Statistics::Histogram::Binning binning = Statistics::Histogram::Binning::UNIFORM;
};

struct OrderImbalanceStatsConfig {
    OrderImbalanceStats::PriceType priceType = OrderImbalanceStats::PriceType::MID;
    double minImbalance = -1.0;
    double maxImbalance = 1.0;
    size_t numBins = 20;
    Statistics::Histogram::Binning binning = Statistics::Histogram::Binning::UNIFORM;
};

struct MonitorOutputsAnalyzerConfig {
    enum class OrderBookStatsAccumulationMode { ALL, EACH_TRADE, EACH_EVENT, LEVEL_ONE_TICK };
    OrderBookStatsAccumulationMode statsAccumulationMode = OrderBookStatsAccumulationMode::EACH_TRADE;
    bool debugMode = false;
};

struct OrderBookDerivedStatsConfig {
    OrderDepthProfileConfig orderDepthProfileConfig = OrderDepthProfileConfig();
    OrderFlowMemoryStatsConfig orderFlowMemoryStatsConfig = OrderFlowMemoryStatsConfig();
    PriceReturnScalingStatsConfig priceReturnScalingStatsConfig = PriceReturnScalingStatsConfig();
    SpreadStatsConfig spreadStatsConfig = SpreadStatsConfig();
    EventTimeStatsConfig eventTimeStatsConfig = EventTimeStatsConfig();
    OrderLifetimeStatsConfig orderLifetimeStatsConfig = OrderLifetimeStatsConfig();
    OrderImbalanceStatsConfig orderImbalanceStatsConfig = OrderImbalanceStatsConfig();
};

std::string toString(const MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode& accumulationMode);
std::ostream& operator<<(std::ostream& out, const MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode& accumulationMode);
}

#endif
