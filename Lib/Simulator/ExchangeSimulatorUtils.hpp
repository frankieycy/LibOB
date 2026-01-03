#ifndef EXCHANGE_SIMULATOR_UTILS_HPP
#define EXCHANGE_SIMULATOR_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"

namespace Simulator {
using namespace Utils;
class IExchangeSimulator;

enum class ExchangeSimulatorState { UNINITIALIZED, READY, RUNNING, FINISHED };
enum class ExchangeSimulatorType { ZERO_INTELLIGENCE, MINIMAL_INTELLIGENCE, NONE };
// types of order events that can be scheduled in the simulator - note the difference between these and Market::OrderEventType,
// the latter being used inside the matching engine to represent actual order events processed by the engine.
enum class OrderEventType { LIMIT_SUBMIT, MARKET_SUBMIT, CANCEL, CANCEL_ID, CANCEL_REPLACE, NONE };

struct OrderBookGridDefinition {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01; // passes down to order event manager
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book for initial book building, spanning $100 around the anchor price by default
};

struct ExchangeSimulatorConfig {
    bool debugMode = false; // passes down to matching engine
    bool debugShowOrderBookPerEvent = false;
    bool resetMatchingEngineMonitorPreSimulation = true;
    size_t monitoredLevels = 100; // passes down to matching engine monitor
    uint64_t randomSeed = 42;
    OrderBookGridDefinition grid;
};

struct ExchangeSimulatorStopCondition {
    ExchangeSimulatorStopCondition(
        const std::optional<uint64_t>& maxTimestamp = std::nullopt,
        const std::optional<uint32_t>& maxNumEvents = std::nullopt) :
        maxTimestamp(maxTimestamp), maxNumEvents(maxNumEvents) {}
    std::optional<uint64_t> maxTimestamp;
    std::optional<uint32_t> maxNumEvents;
    bool check(const IExchangeSimulator& simulator) const;
};

// order event definitions used in the simulator's event scheduler
struct OrderEventBase {
    OrderEventBase() : eventId(0), timestamp(0) {};
    OrderEventBase(const uint64_t eventId, const uint64_t timestamp) :
        eventId(eventId), timestamp(timestamp) {}
    virtual ~OrderEventBase() = default;
    virtual std::string getAsJson() const;
    virtual void submitTo(Market::OrderEventManagerBase& /* orderEventManager */) const {
        Error::LIB_THROW("No implementation for OrderEventBase::submitTo().");
    }
    uint64_t eventId;
    uint64_t timestamp;
    static constexpr OrderEventType eventType = OrderEventType::NONE;
};

struct LimitOrderSubmitEvent : public OrderEventBase {
    LimitOrderSubmitEvent() = delete;
    LimitOrderSubmitEvent(const uint64_t eventId, const uint64_t timestamp, const Market::Side side, const uint32_t quantity, const double price) :
        OrderEventBase(eventId, timestamp), side(side), quantity(quantity), price(price) {}
    virtual std::string getAsJson() const override;
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.submitLimitOrderEvent(side, quantity, price);
    }
    Market::Side side;
    uint32_t quantity;
    double price;
    static constexpr OrderEventType eventType = OrderEventType::LIMIT_SUBMIT;
};

struct MarketOrderSubmitEvent : public OrderEventBase {
    MarketOrderSubmitEvent() = delete;
    MarketOrderSubmitEvent(const uint64_t eventId, const uint64_t timestamp, const Market::Side side, const uint32_t quantity) :
        OrderEventBase(eventId, timestamp), side(side), quantity(quantity) {}
    virtual std::string getAsJson() const override;
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.submitMarketOrderEvent(side, quantity);
    }
    Market::Side side;
    uint32_t quantity;
    static constexpr OrderEventType eventType = OrderEventType::MARKET_SUBMIT;
};

struct OrderCancelEvent : public OrderEventBase {
    OrderCancelEvent() = delete;
    OrderCancelEvent(const uint64_t eventId, const uint64_t timestamp, const Market::Side side, const uint32_t quantity, const double price) :
        OrderEventBase(eventId, timestamp), side(side), quantity(quantity), price(price) {}
    virtual std::string getAsJson() const override;
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelOrders(side, quantity, price);
    }
    Market::Side side;
    uint32_t quantity;
    double price;
    static constexpr OrderEventType eventType = OrderEventType::CANCEL;
};

struct OrderCancelByIdEvent : public OrderEventBase {
    OrderCancelByIdEvent() = delete;
    OrderCancelByIdEvent(const uint64_t eventId, const uint64_t timestamp, const uint64_t orderId) :
        OrderEventBase(eventId, timestamp), orderId(orderId) {}
    virtual std::string getAsJson() const override;
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelOrder(orderId);
    }
    uint64_t orderId;
    static constexpr OrderEventType eventType = OrderEventType::CANCEL_ID;
};

struct OrderCancelAndReplaceEvent : public OrderEventBase {
    OrderCancelAndReplaceEvent() = delete;
    OrderCancelAndReplaceEvent(const uint64_t eventId, const uint64_t timestamp, const uint64_t orderId,
        const std::optional<uint32_t>& modifiedQuantity = std::nullopt, const std::optional<double>& modifiedPrice = std::nullopt) :
        OrderEventBase(eventId, timestamp), orderId(orderId), modifiedQuantity(modifiedQuantity), modifiedPrice(modifiedPrice) {}
    virtual std::string getAsJson() const override;
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelAndReplaceOrder(orderId, modifiedQuantity, modifiedPrice);
    }
    uint64_t orderId;
    std::optional<uint32_t> modifiedQuantity;
    std::optional<double> modifiedPrice;
    static constexpr OrderEventType eventType = OrderEventType::CANCEL_REPLACE;
};

class IEventScheduler {
public:
    virtual ~IEventScheduler() = default;
    virtual std::shared_ptr<OrderEventBase> nextEvent(uint64_t currentTimestamp) = 0;
    virtual bool isExhausted() const { return false; } // TODO: exhaustion check
};

class PerEventScheduler : public IEventScheduler {
public:
    PerEventScheduler(std::function<std::shared_ptr<OrderEventBase>()> generator) :
        myGenerator(std::move(generator)) {}
    std::shared_ptr<OrderEventBase> nextEvent(uint64_t /* currentTimestamp */) override;
private:
    std::function<std::shared_ptr<OrderEventBase>()> myGenerator;
};

class PoissonEventScheduler : public IEventScheduler {
public:
    PoissonEventScheduler(std::function<std::shared_ptr<OrderEventBase>(uint64_t)> generator, double lambda, uint64_t seed = 42) :
        myLambda(lambda), myGenerator(std::move(generator)), myRng(seed), myUniform(0.0, 1.0) {}
    std::shared_ptr<OrderEventBase> nextEvent(uint64_t currentTimestamp) override;
private:
    double myLambda;
    std::function<std::shared_ptr<OrderEventBase>(uint64_t)> myGenerator;
    std::mt19937_64 myRng;
    std::uniform_real_distribution<double> myUniform;
};

class TimeDependentPoissonEventScheduler : public IEventScheduler {}; // TODO

std::string toString(const ExchangeSimulatorState state);
std::string toString(const ExchangeSimulatorType type);
std::string toString(const OrderEventType type);
std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorState state);
std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorType type);
std::ostream& operator<<(std::ostream& out, const OrderEventType type);
std::ostream& operator<<(std::ostream& out, const OrderEventBase& event);
}

#endif
