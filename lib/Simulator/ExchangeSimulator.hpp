#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

namespace Simulator {
using namespace Utils;

class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    IExchangeSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine)
        : myMatchingEngine(matchingEngine) {
        init();
    }
    virtual ~IExchangeSimulator() = default;
    virtual void init();

    std::shared_ptr<Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }

    void setDebugMode(bool debugMode) { myDebugMode = debugMode; }
    void setOrderBookNumLevelsMonitored(const size_t numLevels) {
        myOrderBookNumLevelsMonitored = numLevels;
        if (myMatchingEngineMonitor)
            myMatchingEngineMonitor->setOrderBookNumLevels(numLevels);
    }
    void setAnchorPrice(double anchorPrice) { myAnchorPrice = anchorPrice; }
    void setMinimumPriceTick(double minimumPriceTick) {
        myMinimumPriceTick = minimumPriceTick;
        if (myOrderEventManager)
            myOrderEventManager->setMinimumPriceTick(minimumPriceTick);
    }
    void setMinimumLotSize(uint32_t minimumLotSize) { myMinimumLotSize = minimumLotSize; }

private:
    bool myDebugMode = false;
    size_t myOrderBookNumLevelsMonitored = 10;
    double myAnchorPrice = Consts::NAN_DOUBLE;
    double myMinimumPriceTick = 0.01;
    uint32_t myMinimumLotSize = 1;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
