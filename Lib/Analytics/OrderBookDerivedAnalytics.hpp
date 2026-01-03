#ifndef ORDER_BOOK_DERIVED_ANALYTICS_HPP
#define ORDER_BOOK_DERIVED_ANALYTICS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"

namespace Analytics {
using namespace Utils;

/* Multi-horizon price returns aggregator to study the scaling law of returns:
    Var(return_dt) ~ dt^H with H being the Hurst exponent.
    Each horizon dt yields a sumReturns, sumReturnsSquared etc. */
struct PriceReturnScalingStats {
    std::vector<uint64_t> horizons;
    std::vector<double> sumReturns;
    std::vector<double> sumReturnsSquared;
    std::vector<double> varReturns;
    std::vector<size_t> counts;
};

/* Event time (number of events) between each price tick (mid or best or whatever). */
struct EventTimeStats {
    std::vector<size_t> numEventsSinceLastPriceMove;
    std::map<uint64_t, std::vector<std::shared_ptr<const Market::OrderEventBase>>> eventsSinceLastPriceMove; // map of event time to list of events
    Statistics::Histogram histogram;
};

/* Autocorrelation of trade signs: +1 for buy and -1 for sell. */
struct OrderFlowMemoryStats {
    std::vector<int8_t> tradeSigns;
    Statistics::Autocorrelation<int8_t> tradeSignsACF;
};

/* Average depth profile in price ticks measured from opposite best price. */
struct OrderDepthProfileStats {
    size_t numTicks; // size of the depth profile in price ticks
    size_t numSamples;
    std::vector<double> avgProfileBid;
    std::vector<double> avgProfileAsk;
    std::vector<double> stddevProfileBid;
    std::vector<double> stddevProfileAsk;
};

struct OrderLifetimeStats;
struct ImpactResilienceStats;
}

#endif
