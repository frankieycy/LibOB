#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
using namespace Utils;

enum class ExchangeSimulatorType { ZERO_INTELLIGENCE, MINIMAL_INTELLIGENCE, NULL_EXCHANGE_SIMULATOR_TYPE };

struct OrderBookGridDefinition {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01;
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book
};

class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    IExchangeSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
        myMatchingEngine(matchingEngine) {
        init();
    }
    virtual ~IExchangeSimulator() = default;
    virtual void init();
    virtual void reset();
    virtual void initOrderBookBuilding() = 0;
    virtual void evolveNextOrderEventTick() = 0;
    virtual void runSimulation(const uint64_t numOrderEvents) = 0;

    std::shared_ptr<Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    bool isDebugMode() const { return myDebugMode; }
    size_t getOrderBookNumLevelsMonitored() const { return myOrderBookNumLevelsMonitored; }
    double getAnchorPrice() const { return myOrderBookGridDefinition.anchorPrice; }
    uint32_t getNumGrids() const { return myOrderBookGridDefinition.numGrids; }
    const OrderBookGridDefinition& getOrderBookGridDefinition() const { return myOrderBookGridDefinition; }

    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }
    void setOrderBookNumLevelsMonitored(const size_t numLevels) {
        myOrderBookNumLevelsMonitored = numLevels;
        if (myMatchingEngineMonitor)
            myMatchingEngineMonitor->setOrderBookNumLevels(numLevels);
    }
    void setAnchorPrice(const double anchorPrice) { myOrderBookGridDefinition.anchorPrice = anchorPrice; }
    void setNumGrids(const uint32_t numGrids) { myOrderBookGridDefinition.numGrids = numGrids; }
    void setOrderBookGridDefinition(const OrderBookGridDefinition& constraints) {
        myOrderBookGridDefinition = constraints;
        if (myOrderEventManager)
            myOrderEventManager->setMinimumPriceTick(constraints.minPriceTick);
    }

private:
    bool myDebugMode = false;
    size_t myOrderBookNumLevelsMonitored = 10;
    OrderBookGridDefinition myOrderBookGridDefinition;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
