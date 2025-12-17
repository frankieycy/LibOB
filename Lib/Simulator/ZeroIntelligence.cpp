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
    // TODO: zero intelligence event generation logic
    return std::make_shared<OrderEventBase>();
}
}

#endif
