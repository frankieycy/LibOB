#ifndef ZERO_INTELLIGENCE_CPP
#define ZERO_INTELLIGENCE_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulator.hpp"
#include "Simulator/ZeroIntelligence.hpp"
namespace Simulator {
using namespace Utils;

ZeroIntelligenceSimulator::ZeroIntelligenceSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    ExchangeSimulatorBase(matchingEngine) {
    init();
}

void ZeroIntelligenceSimulator::init() {
    ExchangeSimulatorBase::init();
    setEventScheduler(makeEventScheduler());
    *getLogger() << Logger::LogLevel::INFO << "[ZeroIntelligenceSimulator] Zero Intelligence simulator initialization complete.";
}

std::shared_ptr<IEventScheduler> ZeroIntelligenceSimulator::makeEventScheduler() const {
    return nullptr; // TODO: construct event scheduler based on config
}
}

#endif
