#ifndef ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Analytics/OrderBookObservables.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
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
    Utils::Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> orderBookStatisticsCollector;
    Utils::Statistics::TimeSeriesCollector<Exchange::OrderProcessingReport> orderProcessingReportsCollector;
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
    double minSpread = SpreadStats::MinSpread;
    double maxSpread = SpreadStats::MaxSpread;
    size_t numBins = SpreadStats::NumBins;
    Utils::Statistics::Histogram::Binning binning = Utils::Statistics::Histogram::Binning::UNIFORM;
    std::vector<size_t> lags = std::vector<size_t>(SpreadStats::DefaultLags.begin(), SpreadStats::DefaultLags.end());
};

struct EventTimeStatsConfig {
    double minEventTime = EventTimeStats::MinEventTime;
    double maxEventTime = EventTimeStats::MaxEventTime;
    size_t numBins = EventTimeStats::NumBins;
    Utils::Statistics::Histogram::Binning binning = Utils::Statistics::Histogram::Binning::UNIFORM;
    EventTimeStats::PriceType priceType = EventTimeStats::PriceType::MID;
};

struct OrderLifetimeStatsConfig {
    OrderLifetimeStats::PriceSpaceDefinition priceSpace = OrderLifetimeStats::PriceSpaceDefinition::DIFF_TO_OWN_BEST;
    size_t maxTicks = 10; // size of the price buckets in price ticks of the histograms, chosen to be the same as MatchingEngineMonitor::myOrderBookNumLevels default
    double minPriceTick = 0.01;
    double minLifetime = OrderLifetimeStats::MinLifetime;
    double maxLifetime = OrderLifetimeStats::MaxLifetime;
    size_t numBins = OrderLifetimeStats::NumBins;
    Utils::Statistics::Histogram::Binning binning = Utils::Statistics::Histogram::Binning::UNIFORM;
};

struct OrderImbalanceStatsConfig {
    OrderImbalanceStats::PriceType priceType = OrderImbalanceStats::PriceType::MID;
    double minImbalance = OrderImbalanceStats::MinImbalance;
    double maxImbalance = OrderImbalanceStats::MaxImbalance;
    size_t numBins = OrderImbalanceStats::NumBins;
    size_t maxTicks = 10; // size of the price histogram buckets in price ticks, chosen to be the same as MatchingEngineMonitor::myOrderBookNumLevels default
    double minPriceTick = 0.01;
    Utils::Statistics::Histogram::Binning binning = Utils::Statistics::Histogram::Binning::UNIFORM;
};

struct PriceImpactStatsConfig {
    std::vector<uint64_t> horizons = std::vector<uint64_t>(PriceImpactStats::DefaultHorizons.begin(), PriceImpactStats::DefaultHorizons.end());
    double minPriceImpact = PriceImpactStats::MinPriceImpact;
    double maxPriceImpact = PriceImpactStats::MaxPriceImpact;
    size_t numBins = PriceImpactStats::NumBins;
    PriceImpactStats::PriceType priceType = PriceImpactStats::PriceType::MID;
    PriceImpactStats::TradeConditioning tradeConditioning = PriceImpactStats::TradeConditioning::SIGN_ONLY;
    double minPriceTick = 0.01;
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
    PriceImpactStatsConfig priceImpactStatsConfig = PriceImpactStatsConfig();
};
}

template<>
struct Utils::EnumStrings<Analytics::MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode> {
    inline static constexpr std::array<const char*, 4> names = { "All", "EachTrade", "EachEvent", "LevelOneTick" };
};

#endif
