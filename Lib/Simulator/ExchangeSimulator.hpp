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
    uint64_t initialSeed = 42;
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

    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<const Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<const Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    ExchangeSimulatorState getState() const { return myState; }
    const ExchangeSimulatorConfig& getConfig() const { return myConfig; }
    uint64_t getRandomSeed() const { return myConfig.grid.initialSeed; }
    uint64_t getCurrentTimestamp() const { return mySimulationClock->getCurrentTimestamp(); }
    uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return mySimulationClock->tick(elapsedTimeUnit); }
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> getSimulationClock() const { return mySimulationClock; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    void setConfig(const ExchangeSimulatorConfig& config);
    void setRandomSeed(const uint64_t seed) { myConfig.grid.initialSeed = seed; }
    void setSimulationClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& simulationClock) { mySimulationClock = simulationClock; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }

    virtual void init();
    virtual void reset();
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) = 0;
    virtual void advanceByEvent() = 0; // advance by one event (tick in event time)
    virtual void advanceToTimestamp(const Timestamp timestamp) = 0; // advance to timestamp (tick in wall-clock time)
    virtual void simulateByEvent(const uint64_t numEvents) = 0;
    virtual void simulateUntilTimestamp(const Timestamp timestamp) = 0;

private:
    ExchangeSimulatorConfig myConfig;
    ExchangeSimulatorState myState = ExchangeSimulatorState::UNINITIALIZED;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
    // we keep a simulation clock separate from the matching engine's world clock that ticks in engine event time (e.g. ticks upon order submit/process/...)
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> mySimulationClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
};

// class ExchangeSimulatorBase : public IExchangeSimulator {
// public:
//     virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) override;
//     virtual void advanceByEvent() override;
//     virtual void advanceToTimestamp(const Timestamp timestamp) override;
//     virtual void simulateByEvent(const uint64_t numEvents) override;
//     virtual void simulateUntilTimestamp(const Timestamp timestamp) override;
// private:
//     virtual void buildSide(const Market::Side side, const VolumeProfile& volumeProfile);
// };
}

#endif
