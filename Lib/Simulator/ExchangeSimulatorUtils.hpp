#ifndef EXCHANGE_SIMULATOR_UTILS_HPP
#define EXCHANGE_SIMULATOR_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"

namespace Simulator {
using namespace Utils;

enum class ExchangeSimulatorState { UNINITIALIZED, READY, RUNNING, FINISHED };
enum class ExchangeSimulatorType { ZERO_INTELLIGENCE, MINIMAL_INTELLIGENCE, NULL_EXCHANGE_SIMULATOR_TYPE };

struct OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& /* orderEventManager */) const {
        Error::LIB_THROW("No implementation for OrderEventBase::submitTo().");
    }
    std::optional<uint64_t> eventId;
    std::optional<uint64_t> orderId;
    uint64_t timestamp;
    Market::OrderEventType eventType;
};

struct LimitOrderSubmitEvent : public OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.submitLimitOrderEvent(side, quantity, price);
    }
    Market::Side side;
    uint32_t quantity;
    double price;
};

struct MarketOrderSubmitEvent : public OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.submitMarketOrderEvent(side, quantity);
    }
    Market::Side side;
    uint32_t quantity;
};

struct OrderCancelEvent : public OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelOrders(side, quantity, price);
    }
    Market::Side side;
    uint32_t quantity;
    double price;
};

struct OrderCancelByIdEvent : public OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelOrder(orderId);
    }
    uint64_t orderId;
};

struct OrderCancelAndReplaceEvent : public OrderEventBase {
    virtual void submitTo(Market::OrderEventManagerBase& orderEventManager) const override {
        orderEventManager.cancelAndReplaceOrder(orderId, modifiedQuantity, modifiedPrice);
    }
    uint64_t orderId;
    std::optional<uint32_t> modifiedQuantity;
    std::optional<double> modifiedPrice;
};

struct OrderBookGridDefinition {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01; // passes down to order event manager
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book for initial book building
};

struct ExchangeSimulatorConfig {
    bool debugMode = false; // passes down to matching engine
    size_t monitoredLevels = 100; // passes down to matching engine monitor
    uint64_t randomSeed = 42;
    OrderBookGridDefinition grid;
};

class IEventScheduler {
public:
    virtual ~IEventScheduler() = default;
    virtual std::optional<OrderEventBase> nextEvent(uint64_t currentTimestamp) = 0;
};

class PerEventScheduler : public IEventScheduler {
public:
    explicit PerEventScheduler(std::function<OrderEventBase(uint64_t)> generator) :
        myGenerator(std::move(generator)) {}
    std::optional<OrderEventBase> nextEvent(uint64_t currentTimestamp) override;
private:
    std::function<OrderEventBase(uint64_t)> myGenerator;
};

class PoissonEventScheduler : public IEventScheduler {
public:
    explicit PoissonEventScheduler(std::function<OrderEventBase(uint64_t)> generator, double lambda, uint64_t seed = 42) :
        myLambda(lambda), myGenerator(std::move(generator)), myRng(seed), myUniform(0.0, 1.0) {}
    std::optional<OrderEventBase> nextEvent(uint64_t currentTimestamp) override;
private:
    double myLambda;
    std::function<OrderEventBase(uint64_t)> myGenerator;
    std::mt19937_64 myRng;
    std::uniform_real_distribution<double> myUniform;
};

class TimeDependentPoissonEventScheduler : public IEventScheduler {};

std::string toString(const ExchangeSimulatorState state);
std::string toString(const ExchangeSimulatorType type);
std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorState state);
std::ostream& operator<<(std::ostream& out, const ExchangeSimulatorType type);
}

#endif
