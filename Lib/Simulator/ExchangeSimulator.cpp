#ifndef EXCHANGE_SIMULATOR_CPP
#define EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;

ExchangeSimulatorBase::ExchangeSimulatorBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    myMatchingEngine(matchingEngine) {
    init();
}

void ExchangeSimulatorBase::init() {
    Error::LIB_ASSERT(getState() == ExchangeSimulatorState::UNINITIALIZED,
        "[ExchangeSimulatorBase] Simulator state is not uninitialized.");
    if (!myMatchingEngine)
        Error::LIB_THROW("[ExchangeSimulatorBase] Matching engine is null during simulator initialization.");
    myOrderEventManager = std::make_shared<Market::OrderEventManagerBase>(myMatchingEngine);
    myMatchingEngineMonitor = std::make_shared<Analytics::MatchingEngineMonitor>(myMatchingEngine);
    myMatchingEngine->setDebugMode(isDebugMode());
    myMatchingEngineMonitor->setOrderBookNumLevels(getConfig().monitoredLevels);
    myOrderEventManager->setMinimumPriceTick(getConfig().grid.minPriceTick);
    setState(ExchangeSimulatorState::READY);
}

void ExchangeSimulatorBase::reset() {
    // empties out the matching engine and re-initializes the simulator
    Error::LIB_ASSERT(getState() != ExchangeSimulatorState::RUNNING,
        "[ExchangeSimulatorBase] Cannot reset simulator while it is running.");
    if (myMatchingEngine)
        myMatchingEngine->reset();
    getSimulationClock()->reset();
    setState(ExchangeSimulatorState::UNINITIALIZED);
    init();
}

void ExchangeSimulatorBase::setConfig(const ExchangeSimulatorConfig& config) {
    IExchangeSimulator::setConfig(config);
    myMatchingEngine->setDebugMode(isDebugMode());
    myMatchingEngineMonitor->setOrderBookNumLevels(getConfig().monitoredLevels);
    myOrderEventManager->setMinimumPriceTick(getConfig().grid.minPriceTick);
}

void ExchangeSimulatorBase::initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) {
    buildSide(Market::Side::BUY, bidProfile);
    buildSide(Market::Side::SELL, askProfile);
}

void ExchangeSimulatorBase::advanceByEvent() {
    // TODO
}

void ExchangeSimulatorBase::advanceToTimestamp(const uint64_t /* timestamp */) {
    // TODO
}

void ExchangeSimulatorBase::simulateByEvent(const uint64_t /* numEvents */) {
    // TODO
}

void ExchangeSimulatorBase::simulateUntilTimestamp(const uint64_t /* timestamp */) {
    // TODO
}

void ExchangeSimulatorBase::buildSide(const Market::Side /* side */, const VolumeProfile& /* profile */) {
    // TODO
}
}

#endif
