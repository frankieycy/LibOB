#ifndef EXCHANGE_SIMULATOR_HPP
#define EXCHANGE_SIMULATOR_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/ExchangeSimulatorUtils.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    virtual ~IExchangeSimulator() = default;
    ExchangeSimulatorState getState() const { return myState; }
    const ExchangeSimulatorConfig& getConfig() const { return myConfig; }
    bool isDebugMode() const { return myConfig.debugMode; }
    uint64_t getRandomSeed() const { return myConfig.randomSeed; }
    uint64_t getCurrentTimestamp() const { return mySimulationClock->getCurrentTimestamp(); }
    uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return mySimulationClock->tick(elapsedTimeUnit); }
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> getSimulationClock() const { return mySimulationClock; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    void setState(const ExchangeSimulatorState state) { myState = state; }
    void setRandomSeed(const uint64_t seed) { myConfig.randomSeed = seed; }
    void setSimulationClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& simulationClock) { mySimulationClock = simulationClock; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    virtual void init() = 0;
    virtual void reset() = 0;
    virtual void setConfig(const ExchangeSimulatorConfig& config) { myConfig = config; }
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) = 0;
    virtual void advanceByEvent() = 0; // advance by one event (tick in event time)
    virtual void advanceToTimestamp(const uint64_t timestamp) = 0; // advance to timestamp (tick in wall-clock time)
    virtual void advanceByDuration(const uint64_t duration) { advanceToTimestamp(getCurrentTimestamp() + duration); }
    virtual void simulateByEvent(const uint64_t numEvents) = 0;
    virtual void simulateUntilTimestamp(const uint64_t timestamp) = 0;
private:
    ExchangeSimulatorConfig myConfig = ExchangeSimulatorConfig();
    ExchangeSimulatorState myState = ExchangeSimulatorState::UNINITIALIZED;
    // we keep a simulation clock separate from the matching engine's world clock that ticks in engine event time (e.g. ticks upon order submit/process/...)
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> mySimulationClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
};

class ExchangeSimulatorBase : public IExchangeSimulator {
public:
    ExchangeSimulatorBase() = default;
    ExchangeSimulatorBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~ExchangeSimulatorBase() = default;
    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<const Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<const Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    virtual void init() override;
    virtual void reset() override;
    virtual void setConfig(const ExchangeSimulatorConfig& config) override;
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) override;
    virtual void advanceByEvent() override;
    virtual void advanceToTimestamp(const uint64_t timestamp) override;
    virtual void simulateByEvent(const uint64_t numEvents) override;
    virtual void simulateUntilTimestamp(const uint64_t timestamp) override;
private:
    virtual void buildSide(const Market::Side side, const VolumeProfile& profile);
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
};
}

#endif
