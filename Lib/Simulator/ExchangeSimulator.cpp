#ifndef EXCHANGE_SIMULATOR_CPP
#define EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;

void IExchangeSimulator::init() {
    if (!myMatchingEngine)
        Error::LIB_THROW("[IExchangeSimulator] Matching engine is null during simulator initialization.");
    myDebugMode = myMatchingEngine->isDebugMode();
    myOrderEventManager = std::make_shared<Market::OrderEventManagerBase>(myMatchingEngine);
    myMatchingEngineMonitor = std::make_shared<Analytics::MatchingEngineMonitor>(myMatchingEngine);
    myMatchingEngineMonitor->setOrderBookNumLevels(myOrderBookNumLevelsMonitored);
    myOrderBookGridConstraints.minPriceTick = myOrderEventManager->getMinimumPriceTick();
}

void IExchangeSimulator::reset() {
    // empties out the matching engine and re-initializes the simulator
    if (myMatchingEngine)
        myMatchingEngine->reset();
    init();
}
}

#endif
