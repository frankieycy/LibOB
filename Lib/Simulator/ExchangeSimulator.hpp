#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
using namespace Utils;

enum class ExchangeSimulatorState { UNINITIALIZED, READY, RUNNING, FINISHED };
enum class ExchangeSimulatorType { ZERO_INTELLIGENCE, MINIMAL_INTELLIGENCE, NULL_EXCHANGE_SIMULATOR_TYPE };

struct OrderBookGridDefinition {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01; // passes down to order event manager
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book
};

struct ExchangeSimulatorConfig {
    bool debugMode = false; // passes down to matching engine
    size_t monitoredLevels = 100; // passes down to matching engine monitor
    OrderBookGridDefinition grid;
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
    virtual void step() = 0;
    virtual void runSimulation(const uint64_t numEvents) = 0;
    virtual void runUntil(const uint64_t timestamp) = 0;
    virtual ExchangeSimulatorState getState() const noexcept = 0;
    virtual uint64_t getCurrentTimestamp() const noexcept = 0;

    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<const Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<const Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    const ExchangeSimulatorConfig& getConfig() const { return myConfig; }
    void setConfig(const ExchangeSimulatorConfig& config);

private:
    ExchangeSimulatorConfig myConfig;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
