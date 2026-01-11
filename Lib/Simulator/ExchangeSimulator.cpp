#ifndef EXCHANGE_SIMULATOR_CPP
#define EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;

void IExchangeSimulator::setState(const ExchangeSimulatorState state) {
    myState = state;
    if (isDebugMode())
        *myLogger << Logger::LogLevel::DEBUG << "[IExchangeSimulator] Simulator state set to " << myState << ".";
}

void IExchangeSimulator::setLoggerLogFile(const std::string& logFileName, const bool logToConsole, const bool showLogTimestamp) {
    if (logFileName.empty()) {
        *myLogger << Logger::LogLevel::WARNING << "[IExchangeSimulator::setLoggerLogFile] Log file name is empty, logger will not log to file.";
        return;
    }
    myLogger->setLogFile(logFileName, logToConsole, showLogTimestamp);
    *myLogger << Logger::LogLevel::INFO << "[IExchangeSimulator::setLoggerLogFile] Logger log file set to: " << logFileName;
}

void IExchangeSimulator::reset() {
    mySimulationClock->reset();
    myState = ExchangeSimulatorState::UNINITIALIZED;
}

ExchangeSimulatorBase::ExchangeSimulatorBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) :
    myMatchingEngine(matchingEngine) {
    init();
}

void ExchangeSimulatorBase::init() {
    // myEventScheduler is not initialized here as it relies on the implementation details of the derived classes,
    // whose init function shall call setEventScheduler(makeEventScheduler()).
    Error::LIB_ASSERT(getState() == ExchangeSimulatorState::UNINITIALIZED,
        "[ExchangeSimulatorBase] Simulator state is not uninitialized.");
    if (!myMatchingEngine)
        Error::LIB_THROW("[ExchangeSimulatorBase] Matching engine is null during simulator initialization.");
    myOrderEventManager = std::make_shared<Market::OrderEventManagerBase>(myMatchingEngine);
    myMatchingEngineMonitor = std::make_shared<Analytics::MatchingEngineMonitor>(myMatchingEngine);
    myMatchingEngineMonitor->setOrderBookNumLevels(getConfig().monitoredLevels);
    myMatchingEngineMonitor->setMinimumPriceTick(getConfig().grid.minPriceTick);
    myOrderEventManager->setMinimumPriceTick(getConfig().grid.minPriceTick);
    Statistics::RNG::setDeterministicSeed(getRandomSeed());
    setState(ExchangeSimulatorState::READY);
    if (isDebugMode())
        *getLogger() << Logger::LogLevel::DEBUG << "[ExchangeSimulatorBase] Simulator initialization complete with monitored levels "
                     << getConfig().monitoredLevels << " and minimum price tick " << getConfig().grid.minPriceTick << ".";
}

void ExchangeSimulatorBase::reset() {
    // empties out the matching engine and re-initializes the simulator
    Error::LIB_ASSERT(getState() != ExchangeSimulatorState::RUNNING,
        "[ExchangeSimulatorBase] Cannot reset simulator while it is running.");
    myMatchingEngine->reset();
    myOrderEventManager->reset();
    myMatchingEngineMonitor->reset();
    IExchangeSimulator::reset();
}

void ExchangeSimulatorBase::setRandomSeed(const uint seed) {
    IExchangeSimulator::setRandomSeed(seed);
    Statistics::RNG::setDeterministicSeed(seed);
}

void ExchangeSimulatorBase::setConfig(const ExchangeSimulatorConfig& config) {
    IExchangeSimulator::setConfig(config);
    myMatchingEngineMonitor->setOrderBookNumLevels(getConfig().monitoredLevels);
    myMatchingEngineMonitor->setMinimumPriceTick(getConfig().grid.minPriceTick);
    myOrderEventManager->setMinimumPriceTick(getConfig().grid.minPriceTick);
}

void ExchangeSimulatorBase::setMinPriceTick(const double minPriceTick) {
    IExchangeSimulator::setMinPriceTick(minPriceTick);
    myMatchingEngineMonitor->setMinimumPriceTick(minPriceTick);
    myOrderEventManager->setMinimumPriceTick(minPriceTick);
}

void ExchangeSimulatorBase::setMonitoredOrderBookNumLevels(const size_t numLevels) {
    IExchangeSimulator::setMonitoredOrderBookNumLevels(numLevels);
    myMatchingEngineMonitor->setOrderBookNumLevels(numLevels);
}

void ExchangeSimulatorBase::setLoggerLogFile(const std::string& logFileName, const bool logToConsole, const bool showLogTimestamp) {
    IExchangeSimulator::setLoggerLogFile(logFileName, logToConsole, showLogTimestamp);
    myMatchingEngine->setLogger(getLogger());
    myOrderEventManager->setLogger(getLogger());
    myMatchingEngineMonitor->setLogger(getLogger());
}

void ExchangeSimulatorBase::initOrderBookBuilding(const VolumeProfile& bidProfile, const VolumeProfile& askProfile) {
    buildSide(Market::Side::BUY, bidProfile);
    buildSide(Market::Side::SELL, askProfile);
}

void ExchangeSimulatorBase::submit(const OrderEventBase& orderEvent) {
    if (isDebugMode())
        *getLogger() << Logger::LogLevel::DEBUG << "[ExchangeSimulatorBase] Submitting order event at timestamp " << getCurrentTimestamp() << ": " << orderEvent;
    orderEvent.submitTo(*myOrderEventManager);
}

bool ExchangeSimulatorBase::stepOneTick() {
    if (auto nextEvent = myEventScheduler->nextEvent(clockTick())) {
        submit(*nextEvent);
        myOrderEventLog.push_back(nextEvent);
        return true;
    }
    return false;
}

void ExchangeSimulatorBase::stepOneEvent() {
    while (true) {
        if (stepOneTick()) // ends when the next tick yields an event
            return;
        if (myEventScheduler->isExhausted()) { // avoid infinite loop
            setState(ExchangeSimulatorState::FINISHED);
            return;
        }
    }
}

void ExchangeSimulatorBase::advanceToTimestamp(const uint64_t timestamp) {
    while (getCurrentTimestamp() < timestamp)
        stepOneTick();
}

void ExchangeSimulatorBase::simulate() {
    Error::LIB_ASSERT(getState() == ExchangeSimulatorState::READY,
        "[ExchangeSimulatorBase] Simulator state is not ready to start simulation.");
    const auto& config = getConfig();
    const bool showOrderBookPerEvent = isDebugMode() && config.debugShowOrderBookPerEvent;
    if (showOrderBookPerEvent)
        *getLogger() << Logger::LogLevel::DEBUG << "[ExchangeSimulatorBase] Order book snapshot at initialization:\n" << *myMatchingEngine;
    if (config.resetMatchingEngineMonitorPreSimulation)
        myMatchingEngineMonitor->reset(true /* keepLastSnapshot */);
    setState(ExchangeSimulatorState::RUNNING);
    while (true) {
        if (checkStopCondition())
            break;
        stepOneTick(); // wall-clock tick
        if (showOrderBookPerEvent)
            *getLogger() << Logger::LogLevel::DEBUG << "[ExchangeSimulatorBase] Order book snapshot at timestamp " << getCurrentTimestamp() << ":\n" << *myMatchingEngine;
    }
    setState(ExchangeSimulatorState::FINISHED);
}

void ExchangeSimulatorBase::buildSide(const Market::Side side, const VolumeProfile& profile) {
    const double sign = side == Market::Side::BUY ? -1.0 : 1.0;
    for (size_t i = 0; i < getNumGrids(); ++i) {
        const double price = getAnchorPrice() + sign * i * getMinPriceTick();
        const uint32_t quantity = profile(i);
        if (quantity > 0 && price > 0.0)
            myOrderEventManager->submitLimitOrderEvent(side, quantity, price);
    }
}
}

#endif
