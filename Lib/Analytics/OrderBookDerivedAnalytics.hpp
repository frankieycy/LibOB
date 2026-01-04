#ifndef ORDER_BOOK_DERIVED_ANALYTICS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_HPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookObservables.hpp"

/* Structs to store down calculated analytics derived from matching engine monitor outputs. */
namespace Analytics {
using namespace Utils;
struct OrderDepthProfileConfig;
struct PriceReturnScalingStatsConfig;

/* All derived stats structs must inherit from the IOrderBookDerivedStats interface. */
struct IOrderBookDerivedStats {
    virtual ~IOrderBookDerivedStats() = default;
    virtual void init() = 0; // state consistency checks for the internal configs and data members
    virtual void reset() = 0; // clear all accumulated data
    virtual void compute() = 0; // compute final statistics from the accumulated data
    // optionally provide set(...) and accumulate(...) methods for configuring and accumulating data
};

/* Average depth profile in price ticks measured from opposite best price. */
struct OrderDepthProfileStats : public IOrderBookDerivedStats {
    enum class DepthNormalization { BY_TOTAL_DEPTH, BY_BEST_LEVEL, UNNORMALIZED, NONE };
    enum class PriceSpaceDefinition { DIFF_TO_MID, DIFF_TO_OWN_BEST, DIFF_TO_OPPOSITE_BEST, NONE };
    size_t getNumSnapshots() const { return numSnapshots; }
    bool isCountMissingLevels() const { return countMissingLevels; }
    DepthNormalization getNormalization() const { return normalization; }
    PriceSpaceDefinition getPriceSpaceDefinition() const { return priceSpace; }

    void set(const OrderDepthProfileConfig& config);
    void accumulate(const OrderBookTopLevelsSnapshot& /* snapshot */) {} // TODO
    virtual void init() override;
    virtual void reset() override;
    virtual void compute() override {}

    std::vector<double> avgBid, avgAsk;
    std::vector<double> stdBid, stdAsk;
    std::vector<size_t> nonZeroCountBid, nonZeroCountAsk;
    size_t maxTicks = 0; // size of the depth profile in price ticks

private:
    size_t numSnapshots = 0; // number of snapshots used to compute the profile
    std::vector<double> sumBid, sumAsk;
    std::vector<double> sumSqBid, sumSqAsk;

    DepthNormalization normalization = DepthNormalization::NONE;
    PriceSpaceDefinition priceSpace = PriceSpaceDefinition::NONE;
    bool countMissingLevels = true;
};

/* Autocorrelation of trade signs: +1 for buy and -1 for sell. */
struct OrderFlowMemoryStats : public IOrderBookDerivedStats {
    void accumulate(const int8_t tradeSign);
    virtual void init() override {} // TODO
    virtual void reset() override;
    virtual void compute() override {}

    Statistics::Autocorrelation<int8_t> tradeSignsACF;
};

/* Spread statistics in space (histogram) and time (autocorrelation). */
struct SpreadStats : public IOrderBookDerivedStats {
    void accumulate(const double spread);
    virtual void init() override {} // TODO
    virtual void reset() override;
    virtual void compute() override {}

    Statistics::Histogram spreadHistogram;
    Statistics::Autocorrelation<double> spreadACF;
};

/* Multi-horizon price returns aggregator to study the scaling law of returns:
    Var(return_dt) ~ dt^H with H being the Hurst exponent.
    Each horizon dt yields a sumReturns, sumSqReturns etc. */
struct PriceReturnScalingStats : public IOrderBookDerivedStats {
    enum class PriceType { LAST_TRADE, MID, MICRO, NONE };
    bool isLogReturns() const { return logReturns; }
    PriceType getPriceType() const { return priceType; }

    void set(const PriceReturnScalingStatsConfig& config);
    virtual void init() override;
    virtual void reset() override;
    virtual void compute() override {}

    std::vector<uint64_t> horizons;
    std::vector<double> sumReturns;
    std::vector<double> sumSqReturns;
    std::vector<size_t> counts;

private:
    PriceType priceType = PriceType::NONE;
    bool logReturns = true;
};

/* Event time (number of events) between each price tick (mid or best or whatever). */
struct EventTimeStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void reset() override {}
    virtual void compute() override {}

    Statistics::Histogram eventsBetweenPriceMoves;
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
