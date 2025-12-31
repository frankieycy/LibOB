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
using OrderEventLog = std::vector<std::shared_ptr<const OrderEventBase>>;

class IExchangeSimulator {
public:
    IExchangeSimulator() = default;
    virtual ~IExchangeSimulator() = default;
    ExchangeSimulatorState getState() const { return myState; }
    ExchangeSimulatorConfig& getConfig() { return myConfig; }
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
    void setNumGrids(const uint32_t numGrids) { myConfig.grid.numGrids = numGrids; }
    void setRandomSeed(const uint64_t seed) { myConfig.randomSeed = seed; }
    void setSimulationClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& simulationClock) { mySimulationClock = simulationClock; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    virtual void init() = 0;
    virtual void reset();
    virtual uint32_t getCurrentNumEvents() const = 0;
    virtual std::shared_ptr<const OrderEventBase> getLastEvent() const = 0;
    // debug mode is not passed down into matching engine and others to avoid messages log bloats,
    // and simulator manages its own debug messages log.
    virtual void setDebugMode(const bool debugMode) { myConfig.debugMode = debugMode; }
    virtual void setConfig(const ExchangeSimulatorConfig& config) { myConfig = config; }
    virtual void setMinPriceTick(const double minPriceTick) { myConfig.grid.minPriceTick = minPriceTick; }
    virtual void setMonitoredOrderBookNumLevels(const size_t numLevels) { myConfig.monitoredLevels = numLevels; }
    virtual void setStopCondition(const ExchangeSimulatorStopCondition& stopCondition) { myStopCondition = stopCondition; }
    virtual void setLoggerLogFile(const std::string& logFileName, const bool logToConsole = false, const bool showLogTimestamp = true);
    virtual bool checkStopCondition() const { return myStopCondition.check(*this); }
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) = 0;
    virtual void submit(const OrderEventBase& orderEvent) = 0;
    virtual bool stepOneTick() = 0; // step by one tick (uniform wall-clock time)
    virtual void stepOneEvent() = 0; // step by one event (event time with stochastic arrival intervals)
    virtual void advanceToTimestamp(const uint64_t timestamp) = 0; // keep stepping one tick from the current timestamp to the target timestamp
    virtual void advanceByDuration(const uint64_t duration) { advanceToTimestamp(getCurrentTimestamp() + duration); }
    virtual void simulate() = 0; // simulate in wall-clock time until stop condition is met (may be imposed upon the number of events or timestamp)
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const = 0;
    static constexpr ExchangeSimulatorType ourType = ExchangeSimulatorType::NULL_EXCHANGE_SIMULATOR_TYPE;
private:
    ExchangeSimulatorConfig myConfig = ExchangeSimulatorConfig();
    ExchangeSimulatorStopCondition myStopCondition = ExchangeSimulatorStopCondition();
    ExchangeSimulatorState myState = ExchangeSimulatorState::UNINITIALIZED;
    // we keep a simulation clock separate from the matching engine's world clock that ticks in engine event time (e.g. ticks upon order submit/process/...)
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> mySimulationClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
};

/* Exchange simulator implementation with a matching engine at its core, upon which
   an order event manager and a matching engine monitor are hooked (they are constructed
   from the matching engine). The simulator order event is generated from the event scheduler
   according to some defined process, which may tick in event time or stochastic wall-clock
   time, then sent to the order event manager for processing. The class provides the basic
   interface for time-stepping and simulation but leaves the event scheduler un-initialized,
   which shall be defined in its concrete implementation via makeEventScheduler(). */
class ExchangeSimulatorBase : public IExchangeSimulator {
public:
    ExchangeSimulatorBase() = delete;
    ExchangeSimulatorBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~ExchangeSimulatorBase() = default;
    const OrderEventLog& getOrderEventLog() const { return myOrderEventLog; }
    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    std::shared_ptr<const Market::OrderEventManagerBase> getOrderEventManager() const { return myOrderEventManager; }
    std::shared_ptr<const Analytics::MatchingEngineMonitor> getMatchingEngineMonitor() const { return myMatchingEngineMonitor; }
    std::shared_ptr<const IEventScheduler> getEventScheduler() const { return myEventScheduler; }
    void setEventScheduler(const std::shared_ptr<IEventScheduler>& eventScheduler) { myEventScheduler = eventScheduler; }
    virtual void init() override;
    virtual void reset() override;
    virtual uint32_t getCurrentNumEvents() const override { return myOrderEventLog.size(); }
    virtual std::shared_ptr<const OrderEventBase> getLastEvent() const override { return myOrderEventLog.empty() ? nullptr : myOrderEventLog.back(); }
    virtual void setConfig(const ExchangeSimulatorConfig& config) override;
    virtual void setMinPriceTick(const double minPriceTick) override;
    virtual void setMonitoredOrderBookNumLevels(const size_t numLevels) override;
    virtual void setLoggerLogFile(const std::string& logFileName, const bool logToConsole = false, const bool showLogTimestamp = true) override;
    virtual void initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) override;
    virtual void submit(const OrderEventBase& orderEvent) override;
    virtual bool stepOneTick() override;
    virtual void stepOneEvent() override;
    virtual void advanceToTimestamp(const uint64_t timestamp) override;
    virtual void simulate() override;
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const override { return myMatchingEngine->orderBookSnapshot(out); }
private:
    virtual void buildSide(const Market::Side side, const VolumeProfile& profile);
    virtual std::shared_ptr<IEventScheduler> makeEventScheduler() const { return nullptr; }
    OrderEventLog myOrderEventLog;
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::shared_ptr<Market::OrderEventManagerBase> myOrderEventManager;
    std::shared_ptr<Analytics::MatchingEngineMonitor> myMatchingEngineMonitor;
    std::shared_ptr<IEventScheduler> myEventScheduler;
};
}

#endif
