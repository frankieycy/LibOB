#ifndef ORDER_EVENT_MANAGER_HPP
#define ORDER_EVENT_MANAGER_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "OrderEvent.hpp"
#include "MatchingEngine.hpp"

namespace Market {
class OrderEventManagerBase {
public:
    OrderEventManagerBase() = default;
    OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    const uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return myWorldClock->tick(elapsedTimeUnit); }
    void setPrintOrderBookPerOrderSubmit(const bool printOrderBookPerOrderSubmit) { myPrintOrderBookPerOrderSubmit = printOrderBookPerOrderSubmit; }
    void submitOrderEventToMatchingEngine(const std::shared_ptr<OrderEventBase>& event);
    virtual void onExecutionReport(const Exchange::OrderExecutionReport& report);
    virtual std::shared_ptr<OrderSubmitEvent> createLimitOrderSubmitEvent(const Side side, const uint32_t quantity, const double price);
    virtual std::shared_ptr<OrderSubmitEvent> createMarketOrderSubmitEvent(const Side side, const uint32_t quantity);
    virtual std::shared_ptr<OrderCancelEvent> createOrderCancelEvent(const uint64_t orderId);
    virtual std::shared_ptr<OrderModifyPriceEvent> createOrderModifyPriceEvent(const uint64_t orderId, const double modifiedPrice);
    virtual std::shared_ptr<OrderModifyQuantityEvent> createOrderModifyQuantityEvent(const uint64_t orderId, const double modifiedQuantity);
    std::shared_ptr<OrderSubmitEvent> submitLimitOrderEvent(const Side side, const uint32_t quantity, const double price);
    std::shared_ptr<OrderSubmitEvent> submitMarketOrderEvent(const Side side, const uint32_t quantity);
    std::shared_ptr<OrderCancelEvent> cancelOrder(const uint64_t orderId);
    std::shared_ptr<OrderModifyPriceEvent> modifyOrderPrice(const uint64_t orderId, const double modifiedPrice);
    std::shared_ptr<OrderModifyQuantityEvent> modifyOrderQuantity(const uint64_t orderId, const double modifiedQuantity);
    virtual std::ostream& stateSnapshot(std::ostream& out) const;
private:
    Utils::Counter::IdHandlerBase myOrderIdHandler = Utils::Counter::IdHandlerBase();
    Utils::Counter::IdHandlerBase myEventIdHandler = Utils::Counter::IdHandlerBase();
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> myWorldClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::unordered_map<uint64_t, std::shared_ptr<Market::OrderBase>> myActiveOrders;
    bool mySyncClockWithEngine = false;
    bool myDebugMode = false;
    bool myPrintOrderBookPerOrderSubmit = false;
};

std::ostream& operator<<(std::ostream& out, const OrderEventManagerBase& manager);
}

#endif
