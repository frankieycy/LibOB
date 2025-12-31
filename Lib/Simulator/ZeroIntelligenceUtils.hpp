#ifndef ZERO_INTELLIGENCE_UTILS_HPP
#define ZERO_INTELLIGENCE_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

/* Interfaces for drawing the sizes/prices in the event generation of the zero-intelligence simulator.
   One may specify the marginal distributions by having the explicit parameters in the concrete distribution
   classes, or the conditional distributions dependent on the book shape by having the MatchingEngineMonitor
   as a class member - see the EngineAware classes. */
namespace Simulator {
using namespace Utils;

/* EngineAware samplers have access to the matching engine in real time via the monitor, which may provide
   order book statistics in addition to the book state. Note that the monitor is chosen over the engine so that
   the sample generation may depend on other derived metrics like order imbalance. */
class IEngineAwareSampler {
public:
    IEngineAwareSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        myMonitor(monitor) {}
    virtual ~IEngineAwareSampler() = default;
    double getPriceTick() const { return myMonitor->getMinimumPriceTick(); }
    const Analytics::MatchingEngineMonitor::OrderBookTopLevelsSnapshot& getLastOrderBookTopLevelsSnapshot() const {
        return myMonitor->getLastOrderBookTopLevelsSnapshot();
    }
    const Statistics::TimeSeriesCollector<Analytics::MatchingEngineMonitor::OrderBookStatisticsByTimestamp>& getOrderBookStatistics() const {
        return myMonitor->getOrderBookStatistics();
    }
protected:
    std::shared_ptr<const Analytics::MatchingEngineMonitor> myMonitor;
};

////////////////////////////////////// Event rate samplers //////////////////////////////////////

class IOrderEventRateSampler {
public:
    virtual ~IOrderEventRateSampler() = default;
    virtual double sample() const = 0;
};

class IEngineAwareOrderEventRateSampler : public IOrderEventRateSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderEventRateSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IOrderEventRateSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderEventRateSampler() = default;
};

class ConstantOrderEventRateSampler : public IOrderEventRateSampler {
public:
    ConstantOrderEventRateSampler(const double rate) : myRate(rate) {}
    virtual ~ConstantOrderEventRateSampler() = default;
    virtual double sample() const override { return myRate; }
private:
    double myRate; // events per unit time
};

/* Define totalBidSize = (sum of bid order sizes between bestAsk - myMinOffsetTicks and bestAsk - myMaxOffsetTicks),
    and totalAskSize = (sum of ask order sizes between bestBid + myMinOffsetTicks and bestBid + myMaxOffsetTicks).
    Model the event rate as myRate * (totalBidSize + totalAskSize). */
class OrderEventRateSamplerProportionalTotalSizeFromOppositeBest : public IEngineAwareOrderEventRateSampler {
public:
OrderEventRateSamplerProportionalTotalSizeFromOppositeBest(
        const double rate,
        const uint32_t minOffsetTicks,
        const uint32_t maxOffsetTicks,
        const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IEngineAwareOrderEventRateSampler(monitor),
        myRate(rate),
        myMinOffsetTicks(minOffsetTicks),
        myMaxOffsetTicks(maxOffsetTicks) {}
    virtual ~OrderEventRateSamplerProportionalTotalSizeFromOppositeBest() = default;
    virtual double sample() const override;
private:
    double myRate; // events per unit time per unit size
    uint32_t myMinOffsetTicks;
    uint32_t myMaxOffsetTicks;
};

////////////////////////////////////// Order side samplers //////////////////////////////////////

class IOrderSideSampler {
public:
    virtual ~IOrderSideSampler() = default;
    virtual Market::Side sample() const = 0;
};

class IEngineAwareOrderSideSampler : public IOrderSideSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderSideSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IOrderSideSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderSideSampler() = default;
};

class UniformOrderSideSampler : public IOrderSideSampler {
public:
    virtual ~UniformOrderSideSampler() = default;
    virtual Market::Side sample() const override {
        return Statistics::getRandomUniform01(true) < 0.5 ? Market::Side::BUY : Market::Side::SELL;
    }
};

/* Define totalBidSize = (sum of bid order sizes between bestAsk - myMinOffsetTicks and bestAsk - myMaxOffsetTicks),
    and totalAskSize = (sum of ask order sizes between bestBid + myMinOffsetTicks and bestBid + myMaxOffsetTicks).
    Model the order side as a Bernoulli with the bid probability being totalBidSize / (totalBidSize + totalAskSize). */
class OrderSideSamplerProportionalTotalSizeFromOppositeBest : public IEngineAwareOrderSideSampler {
public:
    OrderSideSamplerProportionalTotalSizeFromOppositeBest(
        const uint32_t minOffsetTicks,
        const uint32_t maxOffsetTicks,
        const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IEngineAwareOrderSideSampler(monitor),
        myMinOffsetTicks(minOffsetTicks),
        myMaxOffsetTicks(maxOffsetTicks) {}
    virtual ~OrderSideSamplerProportionalTotalSizeFromOppositeBest() = default;
    virtual Market::Side sample() const override;
private:
    uint32_t myMinOffsetTicks;
    uint32_t myMaxOffsetTicks;
};

////////////////////////////////////// Order size samplers //////////////////////////////////////

class IOrderSizeSampler {
public:
    virtual ~IOrderSizeSampler() = default;
    virtual uint32_t sample(const Market::Side side) const = 0;
};

class IEngineAwareOrderSizeSampler : public IOrderSizeSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderSizeSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IOrderSizeSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderSizeSampler() = default;
};

class NullOrderSizeSampler : public IOrderSizeSampler {
public:
    virtual ~NullOrderSizeSampler() = default;
    virtual uint32_t sample(const Market::Side /* side */) const override {
        Error::LIB_THROW("[NullOrderSizeSampler] sample() cannot be called.");
        return 0;
    }
};

class ConstantOrderSizeSampler : public IOrderSizeSampler {
public:
    ConstantOrderSizeSampler(const uint32_t size) : mySize(size) {}
    virtual ~ConstantOrderSizeSampler() = default;
    virtual uint32_t sample(const Market::Side /* side */) const override { return mySize; }
private:
    uint32_t mySize;
};

class UniformOrderSizeSampler : public IOrderSizeSampler {
public:
    UniformOrderSizeSampler(const uint32_t minSize, const uint32_t maxSize) :
        myMinSize(minSize), myMaxSize(maxSize) {}
    virtual ~UniformOrderSizeSampler() = default;
    virtual uint32_t sample(const Market::Side /* side */) const override {
        return Statistics::getRandomUniformInt(myMinSize, myMaxSize, true);
    }
private:
    uint32_t myMinSize;
    uint32_t myMaxSize;
};

////////////////////////////////////// Order price placement samplers //////////////////////////////////////

class IOrderPricePlacementSampler {
public:
    virtual ~IOrderPricePlacementSampler() = default;
    virtual double sample(const Market::Side side) const = 0;
};

class IEngineAwareOrderPricePlacementSampler : public IOrderPricePlacementSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderPricePlacementSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IOrderPricePlacementSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderPricePlacementSampler() = default;
};

class NullOrderPricePlacementSampler : public IOrderPricePlacementSampler {
public:
    virtual ~NullOrderPricePlacementSampler() = default;
    virtual double sample(const Market::Side /* side */) const override {
        Error::LIB_THROW("[NullOrderPricePlacementSampler] sample() cannot be called.");
        return Utils::Consts::NAN_DOUBLE;
    }
};

/* Model the bid price placement sampled uniformly between bestAsk - myMinOffsetTicks and bestAsk + myMaxOffsetTicks,
    and the ask price placement between bestBid + myMinOffsetTicks and bestBid + myMaxOffsetTicks. */
class OrderPricePlacementSamplerUniformFromOppositeBest : public IEngineAwareOrderPricePlacementSampler {
public:
    OrderPricePlacementSamplerUniformFromOppositeBest(
        const uint32_t minOffsetTicks,
        const uint32_t maxOffsetTicks,
        const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IEngineAwareOrderPricePlacementSampler(monitor),
        myMinOffsetTicks(minOffsetTicks),
        myMaxOffsetTicks(maxOffsetTicks) {}
    virtual ~OrderPricePlacementSamplerUniformFromOppositeBest() = default;
    virtual double sample(const Market::Side side) const override;
private:
    uint32_t myMinOffsetTicks;
    uint32_t myMaxOffsetTicks;
};

////////////////////////////////////// Order cancellation samplers //////////////////////////////////////

struct OrderCancelSpec {
    uint32_t quantity;
    double price;
};

class IOrderCancellationSampler {
public:
    virtual ~IOrderCancellationSampler() = default;
    virtual std::optional<OrderCancelSpec> sample(const Market::Side side) const = 0; // optional to allow for empty book
};

class IEngineAwareOrderCancellationSampler : public IOrderCancellationSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderCancellationSampler(const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IOrderCancellationSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderCancellationSampler() = default;
};

class NullOrderCancellationSampler : public IOrderCancellationSampler {
public:
    virtual ~NullOrderCancellationSampler() = default;
    virtual std::optional<OrderCancelSpec> sample(const Market::Side /* side */) const override { return std::nullopt; }
};

/* Model the bid order cancellation with a sampled price between bestAsk - myMinOffsetTicks and bestAsk + myMaxOffsetTicks,
    and the ask order cancellation between bestBid + myMinOffsetTicks and bestBid + myMaxOffsetTicks, all with a constant size. */
class OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest : public IEngineAwareOrderCancellationSampler {
public:
    OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest(
        const uint32_t size,
        const uint32_t minOffsetTicks,
        const uint32_t maxOffsetTicks,
        const std::shared_ptr<const Analytics::MatchingEngineMonitor>& monitor) :
        IEngineAwareOrderCancellationSampler(monitor),
        mySize(size),
        myMinOffsetTicks(minOffsetTicks),
        myMaxOffsetTicks(maxOffsetTicks) {}
    virtual ~OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest() = default;
    virtual std::optional<OrderCancelSpec> sample(const Market::Side side) const override;
private:
    uint32_t mySize;
    uint32_t myMinOffsetTicks;
    uint32_t myMaxOffsetTicks;
};
}

#endif
