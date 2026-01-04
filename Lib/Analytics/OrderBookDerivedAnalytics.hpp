#ifndef ORDER_BOOK_DERIVED_ANALYTICS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_HPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookObservables.hpp"

/* Structs to store down calculated analytics derived from matching engine monitor outputs. */
namespace Analytics {
using namespace Utils;

struct OrderBookTraces {
    Statistics::TimeSeriesCollector<OrderBookStatisticsByTimestamp> orderBookStatisticsCollector;
    Statistics::TimeSeriesCollector<Exchange::OrderProcessingReport> orderProcessingReportsCollector;
};

/* Multi-horizon price returns aggregator to study the scaling law of returns:
    Var(return_dt) ~ dt^H with H being the Hurst exponent.
    Each horizon dt yields a sumReturns, sumReturnsSquared etc. */
struct PriceReturnScalingStats {
    enum class PriceType { LAST_TRADE, MID, MICRO, NONE };
    std::vector<uint64_t> horizons;
    std::vector<double> sumReturns;
    std::vector<double> sumReturnsSquared;
    std::vector<size_t> counts;
    bool logReturns = true;
    PriceType priceType = PriceType::NONE;
};

/* Event time (number of events) between each price tick (mid or best or whatever). */
struct EventTimeStats {
    Statistics::Histogram eventsBetweenPriceMoves;
};

/* Autocorrelation of trade signs: +1 for buy and -1 for sell. */
struct OrderFlowMemoryStats {
    Statistics::Autocorrelation<int8_t> tradeSignsACF;
};

/* Average depth profile in price ticks measured from opposite best price. */
struct OrderDepthProfileStats {
    enum class DepthNormalization { BY_TOTAL_DEPTH, BY_BEST_LEVEL, NONE };
    std::vector<double> avgBidProfile;
    std::vector<double> avgAskProfile;
    std::vector<double> stddevBidProfile;
    std::vector<double> stddevAskProfile;
    size_t numTicks = 0; // size of the depth profile in price ticks
    size_t numSamples = 0; // averaged over how many samples
    DepthNormalization normalization = DepthNormalization::NONE;
};

/* Spread statistics in space (histogram) and time (autocorrelation). */
struct SpreadStats {
    Statistics::Histogram spreadHistogram;
    Statistics::Autocorrelation<double> spreadACF;
};

struct OrderLifetimeStats;
struct PriceImpactStats;
}

#endif
