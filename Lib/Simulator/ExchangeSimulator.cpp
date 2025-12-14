#ifndef EXCHANGE_SIMULATOR_CPP
#define EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;

void IExchangeSimulator::setConfig(const ExchangeSimulatorConfig& config) {
    myConfig = config;
    myMatchingEngine->setDebugMode(myConfig.debugMode);
    myMatchingEngineMonitor->setOrderBookNumLevels(myConfig.monitoredLevels);
    myOrderEventManager->setMinimumPriceTick(myConfig.grid.minPriceTick);
}

void IExchangeSimulator::init() {
    if (!myMatchingEngine)
        Error::LIB_THROW("[IExchangeSimulator] Matching engine is null during simulator initialization.");
    myOrderEventManager = std::make_shared<Market::OrderEventManagerBase>(myMatchingEngine);
    myMatchingEngineMonitor = std::make_shared<Analytics::MatchingEngineMonitor>(myMatchingEngine);
    myMatchingEngine->setDebugMode(myConfig.debugMode);
    myMatchingEngineMonitor->setOrderBookNumLevels(myConfig.monitoredLevels);
    myOrderEventManager->setMinimumPriceTick(myConfig.grid.minPriceTick);
}

void IExchangeSimulator::reset() {
    // empties out the matching engine and re-initializes the simulator
    if (myState == ExchangeSimulatorState::RUNNING)
        Error::LIB_THROW("[IExchangeSimulator] Cannot reset simulator while it is running.");
    if (myMatchingEngine)
        myMatchingEngine->reset();
    init();
}
}

#endif
