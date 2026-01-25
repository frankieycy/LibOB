#ifndef ORDER_BOOK_DERIVED_ANALYTICS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Analytics/OrderBookObservables.hpp"

/* Structs to store down calculated analytics derived from matching engine monitor outputs. */
namespace Analytics {
using namespace Utils;
struct OrderDepthProfileConfig;
struct OrderFlowMemoryStatsConfig;
struct PriceReturnScalingStatsConfig;
struct SpreadStatsConfig;
struct EventTimeStatsConfig;
struct OrderLifetimeStatsConfig;
struct OrderImbalanceStatsConfig;
struct PriceImpactStatsConfig;

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
    enum class PriceType { MID, MICRO, NONE };
    static constexpr double MinEventTime = 0.0;
    static constexpr double MaxEventTime = 1000.0;
    static constexpr size_t NumBins = 1000;
    PriceType getPriceType() const { return priceType; }

    void set(const EventTimeStatsConfig& config);
    void accumulate(const OrderBookStatisticsByTimestamp& stats);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    size_t numPriceTicks = 0;
    double meanPriceTicks = 0.0;
    double varPriceTicks = 0.0;
    Statistics::Histogram eventsBetweenPriceMoves;

private:
    // accumulated for each snapshot
    std::vector<double> prices;
    std::vector<size_t> eventCounts;
    std::vector<uint64_t> timestamps;

    PriceType priceType = PriceType::NONE;
};

/* Limit order average lifetime to cancellation (function of price) or execution (function of price and size). */
struct OrderLifetimeStats : public IOrderBookDerivedStats {
    enum class PriceSpaceDefinition { DIFF_TO_MID, DIFF_TO_OWN_BEST, DIFF_TO_OPPOSITE_BEST, NONE };
    enum class OrderDeathType { CANCEL, EXECUTE };
    static constexpr double MinLifetime = 0.0;
    static constexpr double MaxLifetime = 1000.0;
    static constexpr size_t NumBins = 1000;

    /* Order birth entry where upon death, delete the entry from the births map immediately and
        append the lifetime to the histograms accordingly. */
    struct OrderBirth {
        OrderBirth() = delete;
        OrderBirth(uint64_t timestamp, Market::Side side, uint32_t quantity, double price, double refPrice, long long priceDiffTicks) :
            timestamp(timestamp), side(side), quantityAlive(quantity), price(price), refPrice(refPrice), priceDiffTicks(priceDiffTicks) {}
        uint64_t timestamp;
        Market::Side side;
        uint32_t quantityAlive; // quantity alive that has not yet been executed, reduced upon executions
        double price;
        double refPrice; // reference price (mid, own best, opposite best)
        long long priceDiffTicks; // price difference in ticks to the reference price
    };

    void set(const OrderLifetimeStatsConfig& config);
    // ticks the current timestamp and reference price based on the statistics by timestamp
    void accumulate(const OrderBookStatisticsByTimestamp& stats, const Market::Side side);
    void onOrderPlacement(const uint64_t orderId, const Market::Side side, const uint32_t quantity, const double price);
    void onOrderCancel(const uint64_t orderId);
    void onOrderPartialCancel(const uint64_t orderId, const uint32_t cancelQuantity);
    void onOrderExecute(const uint64_t orderId, const uint32_t executedQuantity);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    std::unordered_map<uint64_t, OrderBirth> orderBirths;
    std::vector<double> meanLifetimeToCancelByBidPriceBucket;
    std::vector<double> meanLifetimeToCancelByAskPriceBucket;
    std::vector<double> meanLifetimeToExecuteByBidPriceBucket;
    std::vector<double> meanLifetimeToExecuteByAskPriceBucket;
    std::vector<Statistics::Histogram> lifetimeToCancelByBidPriceBucket;
    std::vector<Statistics::Histogram> lifetimeToCancelByAskPriceBucket;
    std::vector<Statistics::Histogram> lifetimeToExecuteByBidPriceBucket;
    std::vector<Statistics::Histogram> lifetimeToExecuteByAskPriceBucket;

private:
    PriceSpaceDefinition priceSpace = PriceSpaceDefinition::NONE;
    size_t maxTicks = 0; // size of the price buckets in price ticks of the histograms
    double minPriceTick = 0.01;
    uint64_t currentTimestamp = 0;
    double currentRefPrice = Utils::Consts::NAN_DOUBLE;
    std::optional<double> minLifetime;
    std::optional<double> maxLifetime;
    std::optional<size_t> numBins;
};

/* Price ticks stats (e.g. next price tick or up-tick probability) conditional on the instantaneous order imbalance. */
struct OrderImbalanceStats : public IOrderBookDerivedStats {
    // TODO: extend the immediate price tick to multi-horizon price ticks (ref. PriceImpactStats)
    enum class PriceType { MID, MICRO, NONE };
    static constexpr double MinImbalance = -1.0;
    static constexpr double MaxImbalance = 1.0;
    static constexpr size_t NumBins = 5;
    PriceType getPriceType() const { return priceType; }

    void set(const OrderImbalanceStatsConfig& config);
    void accumulate(const OrderBookStatisticsByTimestamp& stats);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    Statistics::Histogram orderImbalanceHistogram;
    std::vector<double> meanNextPriceTickByOrderImbalanceBucket;
    std::vector<Statistics::Histogram> nextPriceTickByOrderImbalanceBucket; // immediate next price tick conditional on order imbalance

private:
    // accumulated for each snapshot
    std::vector<double> prices;
    std::vector<double> imbalances;
    std::vector<uint64_t> timestamps;

    PriceType priceType = PriceType::NONE;
    size_t maxTicks = 10; // size of the price histogram buckets in price ticks
    double minPriceTick = 0.01;
};

/* Price impact stats conditional on the trade sign or size. Formally, we define "price impact" as the expectation of
    the price tick change over some future timeframe (in trade event time unit) given a certain trade sign or size. */
struct PriceImpactStats : public IOrderBookDerivedStats {
    static constexpr std::array<uint64_t, 11> DefaultHorizons{ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    static constexpr size_t NumTradeBuckets_SignOnly = 2; // buy vs sell
    static constexpr size_t NumTradeBuckets_SizeBucketed = 20; // size in log2 scale
    static constexpr double MinPriceImpact = -100;
    static constexpr double MaxPriceImpact = 100;
    static constexpr size_t NumBins = 200;
    enum class PriceType { MID, MICRO, NONE };
    enum class TradeConditioning { SIGN_ONLY, SIZE_BUCKETED, VOLUME_FRACTION, NONE };

    struct TradeEvent {
        TradeEvent() = delete;
        TradeEvent(uint64_t timestamp, int8_t sign, uint32_t size, double price, double refPrice) :
            timestamp(timestamp), sign(sign), size(size), price(price), refPrice(refPrice) {}
        uint64_t timestamp;
        int8_t sign; // +1 for buy, -1 for sell
        uint32_t size;
        double price;
        double refPrice; // mid or micro price at the trade time
    };

    void set(const PriceImpactStatsConfig& config);
    void accumulate(const OrderBookStatisticsByTimestamp& stats);
    virtual void init() override;
    virtual void clear() override;
    virtual void compute() override;
    virtual std::string getAsJson() const override;

    std::vector<uint64_t> horizons;
    std::vector<TradeEvent> trades;
    std::vector<std::vector<double>> meanPriceImpactByHorizonAndTradeBucket;
    std::vector<std::vector<Statistics::Histogram>> priceImpactByHorizonAndTradeBucket; // price impact (in ticks) at each horizon conditional on trade buckets (e.g. sign only, size buckets)

private:
    PriceType priceType = PriceType::NONE;
    TradeConditioning tradeConditioning = TradeConditioning::NONE;
    double minPriceTick = 0.01;
    std::optional<double> minPriceImpact;
    std::optional<double> maxPriceImpact;
    std::optional<size_t> numBins;
};

std::string toString(const OrderDepthProfileStats::DepthNormalization& normalization);
std::string toString(const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace);
std::string toString(const PriceReturnScalingStats::PriceType& priceType);
std::string toString(const EventTimeStats::PriceType& priceType);
std::string toString(const OrderLifetimeStats::PriceSpaceDefinition& priceSpace);
std::string toString(const OrderLifetimeStats::OrderDeathType& deathType);
std::string toString(const OrderImbalanceStats::PriceType& priceType);
std::string toString(const PriceImpactStats::PriceType& priceType);
std::string toString(const PriceImpactStats::TradeConditioning& tradeConditioning);
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::DepthNormalization& normalization);
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace);
std::ostream& operator<<(std::ostream& out, const PriceReturnScalingStats::PriceType& priceType);
std::ostream& operator<<(std::ostream& out, const EventTimeStats::PriceType& priceType);
std::ostream& operator<<(std::ostream& out, const OrderLifetimeStats::PriceSpaceDefinition& priceSpace);
std::ostream& operator<<(std::ostream& out, const OrderLifetimeStats::OrderDeathType& deathType);
std::ostream& operator<<(std::ostream& out, const OrderImbalanceStats::PriceType& priceType);
std::ostream& operator<<(std::ostream& out, const PriceImpactStats::PriceType& priceType);
std::ostream& operator<<(std::ostream& out, const PriceImpactStats::TradeConditioning& tradeConditioning);
}

#endif
