#ifndef ZERO_INTELLIGENCE_UTILS_HPP
#define ZERO_INTELLIGENCE_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"

/* Interfaces for drawing the sizes/prices in the event generation of the zero-intelligence simulator.
   One may specify the marginal distributions by having the explicit parameters in the concrete distribution
   classes, or the conditional distributions dependent on the book shape by having the MatchingEngineMonitor
   as a class member. */
namespace Simulator {
class IOrderEventRateSampler {
public:
    virtual ~IOrderEventRateSampler() = default;
    virtual double sample() const = 0;
};

class ConstantOrderEventRateSampler : public IOrderEventRateSampler {
public:
    ConstantOrderEventRateSampler(const double rate) : myRate(rate) {}
    virtual ~ConstantOrderEventRateSampler() = default;
    virtual double sample() const override { return myRate; }
private:
    double myRate;
};

class IOrderSideSampler {
public:
    virtual ~IOrderSideSampler() = default;
    virtual Market::Side sample() const = 0;
};

class UniformOrderSideSampler : public IOrderSideSampler {
public:
    virtual ~UniformOrderSideSampler() = default;
    virtual Market::Side sample() const override {
        return Statistics::getRandomUniform01(true) < 0.5 ? Market::Side::BUY : Market::Side::SELL;
    }
};

class IOrderSizeSampler {
public:
    virtual ~IOrderSizeSampler() = default;
    virtual uint32_t sample(Market::Side side) const = 0;
};

class ConstantOrderSizeSampler : public IOrderSizeSampler {
public:
    ConstantOrderSizeSampler(const uint32_t size) : mySize(size) {}
    virtual ~ConstantOrderSizeSampler() = default;
    virtual uint32_t sample(Market::Side /* side */) const override { return mySize; }
private:
    uint32_t mySize;
};

class IOrderPricePlacementSampler {
public:
    virtual ~IOrderPricePlacementSampler() = default;
    virtual double sample(Market::Side side) const = 0;
};

class ConstantOrderPricePlacementSampler : public IOrderPricePlacementSampler {
public:
    ConstantOrderPricePlacementSampler(const double price) : myPrice(price) {}
    virtual ~ConstantOrderPricePlacementSampler() = default;
    virtual double sample(Market::Side /* side */) const override { return myPrice; }
private:
    double myPrice;
};

struct OrderCancelSpec {
    uint32_t quantity;
    double price;
};

class IOrderCancellationSampler {
public:
    virtual ~IOrderCancellationSampler() = default;
    virtual std::optional<OrderCancelSpec> sample(Market::Side side) const = 0; // optional to allow for empty book
};

class NoOrderCancellationSampler : public IOrderCancellationSampler {
public:
    virtual ~NoOrderCancellationSampler() = default;
    virtual std::optional<OrderCancelSpec> sample(Market::Side /* side */) const override { return std::nullopt; }
};
}

#endif
