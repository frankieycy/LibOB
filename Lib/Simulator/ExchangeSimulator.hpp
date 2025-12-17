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
    const ExchangeSimulatorStopCondition& getStopCondition() const { return myStopCondition; }
    bool isDebugMode() const { return myConfig.debugMode; }
    double getAnchorPrice() const { return myConfig.grid.anchorPrice; }
    double getMinPriceTick() const { return myConfig.grid.minPriceTick; }
    uint32_t getNumGrids() const { return myConfig.grid.numGrids; }
    uint64_t getRandomSeed() const { return myConfig.randomSeed; }
    uint64_t getCurrentTimestamp() const { return mySimulationClock->getCurrentTimestamp(); }
    uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return mySimulationClock->tick(elapsedTimeUnit); }
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> getSimulationClock() const { return mySimulationClock; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    void setState(const ExchangeSimulatorState state);
    void setAnchorPrice(const double anchorPrice) { myConfig.grid.anchorPrice = anchorPrice; }
    void setMinPriceTick(const double minPriceTick) { myConfig.grid.minPriceTick = minPriceTick; }
    void setNumGrids(const uint32_t numGrids) { myConfig.grid.numGrids = numGrids; }
    void setRandomSeed(const uint64_t seed) { myConfig.randomSeed = seed; }
    void setSimulationClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& simulationClock) { mySimulationClock = simulationClock; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    virtual void init() = 0;
    virtual void reset() = 0;
    // debug mode is not passed down into matching engine and others to avoid messages log bloats,
    // and simulator manages its own debug messages log.
    virtual void setDebugMode(const bool debugMode) { myConfig.debugMode = debugMode; }
    virtual void setConfig(const ExchangeSimulatorConfig& config) { myConfig = config; }
    virtual void setStopCondition(const ExchangeSimulatorStopCondition& stopCondition) { myStopCondition = stopCondition; }
    virtual bool checkStopCondition() const { return myStopCondition.check(*this); }
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) = 0;
    virtual void submit(const OrderEventBase& orderEvent) = 0;
    virtual bool stepOneTick() = 0; // step by one tick (uniform wall-clock time)
    virtual void stepOneEvent() = 0; // step by one event (event time with stochastic arrival intervals)
    virtual void advanceToTimestamp(const uint64_t timestamp) = 0; // keep stepping one tick from the current timestamp to the target timestamp
    virtual void advanceByDuration(const uint64_t duration) { advanceToTimestamp(getCurrentTimestamp() + duration); }
    virtual void simulate() = 0; // simulate in wall-clock time until stop condition is met (may be imposed upon the number of events or timestamp)
    static constexpr ExchangeSimulatorType ourType = ExchangeSimulatorType::NULL_EXCHANGE_SIMULATOR_TYPE;
private:
    ExchangeSimulatorConfig myConfig = ExchangeSimulatorConfig();
    ExchangeSimulatorStopCondition myStopCondition = ExchangeSimulatorStopCondition();
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
    std::shared_ptr<const IEventScheduler> getEventScheduler() const { return myEventScheduler; }
    void setEventScheduler(const std::shared_ptr<IEventScheduler>& eventScheduler) { myEventScheduler = eventScheduler; }
    virtual void init() override;
    virtual void reset() override;
    virtual void setConfig(const ExchangeSimulatorConfig& config) override;
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) override;
    virtual void submit(const OrderEventBase& orderEvent) override;
    virtual bool stepOneTick() override;
    virtual void stepOneEvent() override;
    virtual void advanceToTimestamp(const uint64_t timestamp) override;
    virtual void simulate() override;
private:
    virtual void buildSide(const Market::Side side, const VolumeProfile& profile);
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
    std::shared_ptr<IEventScheduler> myEventScheduler;
};
}

#endif
