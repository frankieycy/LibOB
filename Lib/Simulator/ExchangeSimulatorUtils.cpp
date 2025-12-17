#ifndef EXCHANGE_SIMULATOR_UTILS_CPP
#define EXCHANGE_SIMULATOR_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulatorUtils.hpp"

namespace Simulator {
std::string toString(const ExchangeSimulatorState state) {
    switch (state) {
        case ExchangeSimulatorState::UNINITIALIZED: return "Uninitialized";
        case ExchangeSimulatorState::READY:         return "Ready";
        case ExchangeSimulatorState::RUNNING:       return "Running";
        case ExchangeSimulatorState::FINISHED:      return "Finished";
        default:                                    return "Null";
    }
}

std::string toString(const ExchangeSimulatorType type) {
    switch (type) {
        case ExchangeSimulatorType::ZERO_INTELLIGENCE:     return "ZeroIntelligence";
        case ExchangeSimulatorType::MINIMAL_INTELLIGENCE:  return "MinimalIntelligence";
        default:                                           return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorState state) { return out << toString(state); }

std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorType type) { return out << toString(type); }

bool ExchangeSimulatorStopCondition::check(const IExchangeSimulator& /* simulator */) const {
    return false; // TODO
}

std::shared_ptr<OrderEventBase> PerEventScheduler::nextEvent(uint64_t /* currentTimestamp */) {
    return myGenerator();
}

std::shared_ptr<OrderEventBase> PoissonEventScheduler::nextEvent(uint64_t currentTimestamp) {
    double u = myUniform(myRng);
    if (u < myLambda)
        return myGenerator(currentTimestamp);
    return nullptr;
}
}

#endif
