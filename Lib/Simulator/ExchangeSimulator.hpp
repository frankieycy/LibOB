#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
using namespace Utils;
using Timestamp = uint64_t;

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

class IEventScheduler {
public:
    virtual ~IEventScheduler() = default;
    virtual std::optional<Market::OrderEventBase> nextEvent(const Timestamp currentTimestamp) = 0;
};

class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    IExchangeSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
        myMatchingEngine(matchingEngine) {
        init();
    }
    virtual ~IExchangeSimulator() = default;

    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<const Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<const Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    const ExchangeSimulatorConfig& getConfig() const { return myConfig; }
    void setConfig(const ExchangeSimulatorConfig& config);

    virtual void init();
    virtual void reset();
    virtual void initOrderBookBuilding() = 0;
    virtual void advanceByEvent() = 0; // advance by one event (tick in event time)
    virtual void advanceToTimestamp(const Timestamp timestamp) = 0; // advance to timestamp (tick in wall-clock time)
    virtual void simulateByEvent(const uint64_t numEvents) = 0;
    virtual void simulateUntilTimestamp(const Timestamp timestamp) = 0;
    virtual ExchangeSimulatorState getState() const = 0;
    virtual Timestamp getCurrentTimestamp() const = 0;
    virtual uint64_t getRandomSeed() const = 0;
    virtual void setRandomSeed(const uint64_t seed) = 0;

private:
    ExchangeSimulatorConfig myConfig;
    ExchangeSimulatorState myState = ExchangeSimulatorState::UNINITIALIZED;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
