#ifndef ZERO_INTELLIGENCE_CPP
#define ZERO_INTELLIGENCE_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulator.hpp"
#include "Simulator/ZeroIntelligence.hpp"

namespace Simulator {
using namespace Utils;

ZeroIntelligenceSimulator::ZeroIntelligenceSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    ExchangeSimulatorBase(matchingEngine) {
    // the base class init has been called inside the base constructor
    init();
}

void ZeroIntelligenceSimulator::init() {
    setEventScheduler(makeEventScheduler());
    *getLogger() << Logger::LogLevel::INFO << "[ZeroIntelligenceSimulator] Zero Intelligence simulator initialization complete.";
}

std::shared_ptr<IEventScheduler> ZeroIntelligenceSimulator::makeEventScheduler() const {
    // simulation in event time so that each tick corresponds to one order event (e.g. limit submit or cancel, market submit)
    return std::make_shared<PerEventScheduler>([this]() -> std::shared_ptr<OrderEventBase> {
        return this->generateNextOrderEvent();
    });
}

std::shared_ptr<OrderEventBase> ZeroIntelligenceSimulator::generateNextOrderEvent() const {
    // zero intelligence event generation logic by sampling in sequence
    // (1) order event type (market submit, limit submit, limit cancel) based on the config rates,
    // (2) order side (buy/sell) based on the side sampler,
    // (3) order size/price based on the size sampler,
    // in which the samplers may be conditioned on the order book state (current or historical).
    const double marketOrderRate = myZIConfig.marketOrderRateSampler->sample();
    const double limitOrderRate = myZIConfig.limitOrderRateSampler->sample();
    const double cancelRate = myZIConfig.cancelRateSampler->sample();
    if (marketOrderRate + limitOrderRate + cancelRate == 0.0)
        return nullptr; // no events to generate
    const std::vector<double> orderEventRates = { marketOrderRate, limitOrderRate, cancelRate };
    // next tick must have one of the events happening, hence conditional sampling
    const size_t eventIndex = Statistics::drawIndexWithRelativeProbabilities(orderEventRates, true);
    if (eventIndex == 0) { // market order submit event
        const Market::Side side = myZIConfig.marketSideSampler->sample();
        const uint32_t size = myZIConfig.marketSizeSampler->sample(side);
        return std::make_shared<MarketOrderSubmitEvent>(getCurrentTimestamp() /* eventId */, getCurrentTimestamp(), side, size);
    } else if (eventIndex == 1) { // limit order submit event
        const Market::Side side = myZIConfig.limitSideSampler->sample();
        const uint32_t size = myZIConfig.limitSizeSampler->sample(side);
        const double price = myZIConfig.limitPriceSampler->sample(side);
        return std::make_shared<LimitOrderSubmitEvent>(getCurrentTimestamp() /* eventId */, getCurrentTimestamp(), side, size, price);
    } else if (eventIndex == 2) { // limit order cancel event
        const Market::Side side = myZIConfig.cancelSideSampler->sample();
        const std::optional<OrderCancelSpec> cancelSpec = myZIConfig.cancelSampler->sample(side);
        if (cancelSpec)
            return std::make_shared<OrderCancelEvent>(getCurrentTimestamp() /* eventId */, getCurrentTimestamp(), side, cancelSpec->quantity, cancelSpec->price);
    }
    return nullptr;
}
}

#endif
