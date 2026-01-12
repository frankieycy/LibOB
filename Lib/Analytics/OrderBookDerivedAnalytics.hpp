#ifndef ORDER_BOOK_DERIVED_ANALYTICS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_HPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookObservables.hpp"

/* Structs to store down calculated analytics derived from matching engine monitor outputs. */
namespace Analytics {
using namespace Utils;
struct OrderDepthProfileConfig;
struct OrderFlowMemoryStatsConfig;
struct PriceReturnScalingStatsConfig;
struct SpreadStatsConfig;

/* All derived stats structs must inherit from the IOrderBookDerivedStats interface. */
struct IOrderBookDerivedStats {
    virtual ~IOrderBookDerivedStats() = default;
    virtual void init() = 0; // state consistency checks for the internal configs and data members
    virtual void clear() = 0; // clear all accumulated data
    virtual void compute() = 0; // compute final statistics from the accumulated data
    virtual std::string getAsJson() const { return "{}"; } // optionally provide JSON serialization
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
    void accumulate(const OrderBookTopLevelsSnapshot& snapshot);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    std::vector<double> avgBid, avgAsk;
    std::vector<double> stdBid, stdAsk;

private:
    size_t numSnapshots = 0; // number of snapshots used to compute the profile
    std::vector<double> sumBid, sumAsk;
    std::vector<double> sumSqBid, sumSqAsk;
    std::vector<size_t> nonZeroCountBid, nonZeroCountAsk;

    DepthNormalization normalization = DepthNormalization::NONE;
    PriceSpaceDefinition priceSpace = PriceSpaceDefinition::NONE;
    size_t maxTicks = 0; // size of the depth profile in price ticks
    double minPriceTick = 0.01;
    bool countMissingLevels = true;
};

/* Autocorrelation of trade signs: +1 for buy and -1 for sell. */
struct OrderFlowMemoryStats : public IOrderBookDerivedStats {
    static constexpr std::array<size_t, 11> DefaultLags{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    void set(const OrderFlowMemoryStatsConfig& config);
    void accumulate(const int8_t tradeSign);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual double getAutocorrelationAt(size_t lag) const { return tradeSignsACF.get(lag); }
    virtual std::string getAsJson() const override;

    size_t numTrades = 0;
    double meanTradeSign = 0.0;
    double varTradeSign = 0.0;
    Statistics::Autocorrelation<int8_t> tradeSignsACF;

private:
    std::vector<size_t> lags;
    std::vector<double> autocorrelations;
};

/* Multi-horizon price returns aggregator to study the scaling law of returns:
    Var(return_dt) ~ dt^H with H being the Hurst exponent.
    Each horizon dt yields a sumReturns, sumSqReturns etc. */
struct PriceReturnScalingStats : public IOrderBookDerivedStats {
    static constexpr std::array<uint64_t, 11> DefaultHorizons{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    enum class PriceType { LAST_TRADE, MID, MICRO, NONE };
    bool isLogReturns() const { return logReturns; }
    PriceType getPriceType() const { return priceType; }

    void set(const PriceReturnScalingStatsConfig& config);
    void accumulate(const OrderBookStatisticsByTimestamp& stats);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    std::vector<uint64_t> horizons;
    std::vector<double> varReturns;

private:
    // accumulated for each snapshot
    std::vector<double> prices;
    std::vector<uint64_t> timestamps;
    // helper accumulators for each horizon
    std::vector<double> sumReturns;
    std::vector<double> sumSqReturns;
    std::vector<size_t> counts;

    PriceType priceType = PriceType::NONE;
    bool logReturns = true;
};

/* Spread statistics in space (histogram) and time (autocorrelation). */
struct SpreadStats : public IOrderBookDerivedStats {
    static constexpr std::array<size_t, 11> DefaultLags{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    static constexpr double MinSpread = 0.0;
    static constexpr double MaxSpread = 1000.0;
    static constexpr size_t NumBins = 100000;
    void set(const SpreadStatsConfig& config);
    void accumulate(const double spread);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual double getAutocorrelationAt(size_t lag) const { return spreadACF.get(lag); }
    virtual std::string getAsJson() const override;

    size_t numSpreads = 0;
    double meanSpread = 0.0;
    double varSpread = 0.0;
    Statistics::Histogram spreadHistogram;
    Statistics::Autocorrelation<double> spreadACF;

private:
    std::vector<size_t> lags;
    std::vector<double> autocorrelations;
};

/* Event time (number of events) between each price tick (mid or best or whatever). */
struct EventTimeStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void clear() override {}
    virtual void compute() override {}

    Statistics::Histogram eventsBetweenPriceMoves;
};

struct OrderLifetimeStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void clear() override {}
    virtual void compute() override {}
};

struct PriceImpactStats : public IOrderBookDerivedStats {
    virtual void init() override {} // TODO
    virtual void clear() override {}
    virtual void compute() override {}
};

std::string toString(const OrderDepthProfileStats::DepthNormalization& normalization);
std::string toString(const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace);
std::string toString(const PriceReturnScalingStats::PriceType& priceType);
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::DepthNormalization& normalization);
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace);
std::ostream& operator<<(std::ostream& out, const PriceReturnScalingStats::PriceType& priceType);
}

#endif
