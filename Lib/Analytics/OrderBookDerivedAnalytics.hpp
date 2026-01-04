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

struct IOrderBookDerivedStats {
    virtual ~IOrderBookDerivedStats() = default;
    virtual void init() = 0;
    virtual void reset() = 0;
    virtual void compute() = 0;
};

/* Multi-horizon price returns aggregator to study the scaling law of returns:
    Var(return_dt) ~ dt^H with H being the Hurst exponent.
    Each horizon dt yields a sumReturns, sumSqReturns etc. */
struct PriceReturnScalingStats : public IOrderBookDerivedStats {
    enum class PriceType { LAST_TRADE, MID, MICRO, NONE };
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}

    std::vector<uint64_t> horizons;
    std::vector<double> sumReturns;
    std::vector<double> sumSqReturns;
    std::vector<size_t> counts;
    bool logReturns = true;
    PriceType priceType = PriceType::NONE;
};

/* Event time (number of events) between each price tick (mid or best or whatever). */
struct EventTimeStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}

    Statistics::Histogram eventsBetweenPriceMoves;
};

/* Autocorrelation of trade signs: +1 for buy and -1 for sell. */
struct OrderFlowMemoryStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}

    Statistics::Autocorrelation<int8_t> tradeSignsACF;
};

/* Average depth profile in price ticks measured from opposite best price. */
struct OrderDepthProfileStats : public IOrderBookDerivedStats {
    enum class DepthNormalization { BY_TOTAL_DEPTH, BY_BEST_LEVEL, NONE };
    enum class PriceSpaceDefinition { DIFF_TO_MID, DIFF_TO_OWN_BEST, DIFF_TO_OPPOSITE_BEST, NONE };
    size_t getNumSnapshots() const { return numSnapshots; }
    void accumulate(const OrderBookTopLevelsSnapshot& /* snapshot */) {} // TODO
    virtual void init() override {}
    virtual void reset() override {}
    virtual void compute() override {}

    std::vector<double> avgBid, avgAsk;
    std::vector<double> stdBid, stdAsk;
    std::vector<size_t> nonZeroCountBid, nonZeroCountAsk;
    size_t maxTicks = 0; // size of the depth profile in price ticks
    bool countMissingLevels = true;
    DepthNormalization normalization = DepthNormalization::NONE;
    PriceSpaceDefinition priceSpace = PriceSpaceDefinition::NONE;

private:
    size_t numSnapshots = 0; // number of snapshots used to compute the profile
    std::vector<double> sumBid, sumAsk;
    std::vector<double> sumSqBid, sumSqAsk;
};

/* Spread statistics in space (histogram) and time (autocorrelation). */
struct SpreadStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}

    Statistics::Histogram spreadHistogram;
    Statistics::Autocorrelation<double> spreadACF;
};

struct OrderLifetimeStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}
};

struct PriceImpactStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}
};
}

#endif
