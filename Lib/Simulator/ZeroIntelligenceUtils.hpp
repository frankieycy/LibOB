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
class IEngineAwareSampler {
public:
    IEngineAwareSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) : myMonitor(monitor) {}
    virtual ~IEngineAwareSampler() = default;
    const Analytics::MatchingEngineMonitor::OrderBookTopLevelsSnapshot& getLastOrderBookTopLevelsSnapshot() const {
        return myMonitor->getLastOrderBookTopLevelsSnapshot();
    }
    const Statistics::TimeSeriesCollector<Analytics::MatchingEngineMonitor::OrderBookStatisticsByTimestamp>& getOrderBookStatistics() const {
        return myMonitor->getOrderBookStatistics();
    }
protected:
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMonitor;
};

class IOrderEventRateSampler {
public:
    virtual ~IOrderEventRateSampler() = default;
    virtual double sample() const = 0;
};

class IEngineAwareOrderEventRateSampler : public IOrderEventRateSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderEventRateSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) :
        IOrderEventRateSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderEventRateSampler() = default;
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

class IEngineAwareOrderSideSampler : public IOrderSideSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderSideSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) :
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

class IOrderSizeSampler {
public:
    virtual ~IOrderSizeSampler() = default;
    virtual uint32_t sample(Market::Side side) const = 0;
};

class IEngineAwareOrderSizeSampler : public IOrderSizeSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderSizeSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) :
        IOrderSizeSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderSizeSampler() = default;
};

class NullOrderSizeSampler : public IOrderSizeSampler {
public:
    virtual ~NullOrderSizeSampler() = default;
    virtual uint32_t sample(Market::Side /* side */) const override {
        Error::LIB_THROW("[NullOrderSizeSampler] sample() cannot be called.");
        return 0;
    }
};

class UniformOrderSizeSampler : public IOrderSizeSampler {
public:
    UniformOrderSizeSampler(const uint32_t minSize, const uint32_t maxSize) :
        myMinSize(minSize), myMaxSize(maxSize) {}
    virtual ~UniformOrderSizeSampler() = default;
    virtual uint32_t sample(Market::Side /* side */) const override {
        return Statistics::getRandomUniformInt(myMinSize, myMaxSize, true);
    }
private:
    uint32_t myMinSize;
    uint32_t myMaxSize;
};

class IOrderPricePlacementSampler {
public:
    virtual ~IOrderPricePlacementSampler() = default;
    virtual double sample(Market::Side side) const = 0;
};

class IEngineAwareOrderPricePlacementSampler : public IOrderPricePlacementSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderPricePlacementSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) :
        IOrderPricePlacementSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderPricePlacementSampler() = default;
};

class NullOrderPricePlacementSampler : public IOrderPricePlacementSampler {
public:
    virtual ~NullOrderPricePlacementSampler() = default;
    virtual double sample(Market::Side /* side */) const override {
        Error::LIB_THROW("[NullOrderPricePlacementSampler] sample() cannot be called.");
        return Utils::Consts::NAN_DOUBLE;
    }
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

class IEngineAwareOrderCancellationSampler : public IOrderCancellationSampler, public IEngineAwareSampler {
public:
    IEngineAwareOrderCancellationSampler(const std::shared_ptr<Analytics::MatchingEngineMonitor>& monitor) :
        IOrderCancellationSampler(), IEngineAwareSampler(monitor) {}
    virtual ~IEngineAwareOrderCancellationSampler() = default;
};

class NullOrderCancellationSampler : public IOrderCancellationSampler {
public:
    virtual ~NullOrderCancellationSampler() = default;
    virtual std::optional<OrderCancelSpec> sample(Market::Side /* side */) const override { return std::nullopt; }
};
}

#endif
