#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"

namespace Simulator {
using namespace Utils;

struct OrderBookGridConstraints {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01;
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book
};

struct OrderBookInitialStateConfig {
    // volume descriptions (in lot unit)
    std::optional<uint32_t> bidSaturationVolume;
    std::optional<uint32_t> askSaturationVolume;
    // price desciptions (in tick unit)
    std::optional<uint32_t> bidCharacteristicDistToBest;
    std::optional<uint32_t> askCharacteristicDistToBest;
    std::optional<uint32_t> bidZeroVolumeDistToAnchor;
    std::optional<uint32_t> askZeroVolumeDistToAnchor;
};

class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    IExchangeSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine)
        : myMatchingEngine(matchingEngine) {
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
    double getAnchorPrice() const { return myOrderBookGridConstraints.anchorPrice; }
    uint32_t getNumGrids() const { return myOrderBookGridConstraints.numGrids; }
    const OrderBookGridConstraints& getOrderBookGridConstraints() const { return myOrderBookGridConstraints; }
    const OrderBookInitialStateConfig& getOrderBookInitialStateConfig() const { return myOrderBookInitialStateConfig; }

    void setDebugMode(bool debugMode) { myDebugMode = debugMode; }
    void setOrderBookNumLevelsMonitored(const size_t numLevels) {
        myOrderBookNumLevelsMonitored = numLevels;
        if (myMatchingEngineMonitor)
            myMatchingEngineMonitor->setOrderBookNumLevels(numLevels);
    }
    void setAnchorPrice(double anchorPrice) { myOrderBookGridConstraints.anchorPrice = anchorPrice; }
    void setNumGrids(uint32_t numGrids) { myOrderBookGridConstraints.numGrids = numGrids; }
    void setOrderBookGridConstraints(const OrderBookGridConstraints& constraints) {
        myOrderBookGridConstraints = constraints;
        if (myOrderEventManager)
            myOrderEventManager->setMinimumPriceTick(constraints.minPriceTick);
    }
    void setOrderBookInitialStateConfig(const OrderBookInitialStateConfig& config) { myOrderBookInitialStateConfig = config; }

private:
    bool myDebugMode = false;
    size_t myOrderBookNumLevelsMonitored = 10;
    OrderBookGridConstraints myOrderBookGridConstraints;
    OrderBookInitialStateConfig myOrderBookInitialStateConfig;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
