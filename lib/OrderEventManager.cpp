#ifndef ORDER_EVENT_MANAGER_CPP
#define ORDER_EVENT_MANAGER_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "OrderEvent.hpp"
#include "MatchingEngine.hpp"
#include "OrderEventManager.hpp"

namespace Market {
using namespace Utils;
OrderEventManagerBase::OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("OrderEventManagerBase: matching engine is null.");
    mySyncClockWithEngine = true;
    myMatchingEngine = matchingEngine;
    myWorldClock = matchingEngine->getWorldClock();
    myDebugMode = matchingEngine->isDebugMode();
    matchingEngine->setOrderExecutionCallback([this](const Exchange::OrderExecutionReport& report) { onExecutionReport(report); });
}

void OrderEventManagerBase::submitOrderEventToMatchingEngine(const std::shared_ptr<OrderEventBase>& event) {
    myMatchingEngine->process(event);
    if (myDebugMode && myPrintOrderBookPerOrderSubmit)
        *myLogger << "[OrderEventManagerBase] Order book state:\n" << *myMatchingEngine;
}

void OrderEventManagerBase::onExecutionReport(const Exchange::OrderExecutionReport& report) {
    if (report.orderExecutionType == Exchange::OrderExecutionType::FILLED || report.orderExecutionType == Exchange::OrderExecutionType::PARTIAL_FILLED) {
        const auto& it = myActiveOrders.find(report.orderId);
        if (it != myActiveOrders.end()) {
            const auto& order = it->second;
            const uint32_t updatedQuantity = order->getQuantity() - report.filledQuantity;
            order->setQuantity(updatedQuantity);
            if (updatedQuantity == 0) {
                order->setOrderState(Market::OrderState::FILLED);
                myActiveOrders.erase(it);
            } else {
                order->setOrderState(Market::OrderState::PARTIAL_FILLED);
            }
        }
    }
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::createLimitOrderSubmitEvent(const Side side, const uint32_t quantity, const double price) {
    const auto& order = std::make_shared<LimitOrder>(myOrderIdHandler.generateId(), myWorldClock->getCurrentTimestamp(), side, quantity, price);
    const auto& event = std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), myWorldClock->getCurrentTimestamp(), order);
    myActiveOrders[order->getId()] = order;
    return event;
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::createMarketOrderSubmitEvent(const Side side, const uint32_t quantity) {
    const auto& order = std::make_shared<MarketOrder>(myOrderIdHandler.generateId(), myWorldClock->getCurrentTimestamp(), side, quantity);
    const auto& event = std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), myWorldClock->getCurrentTimestamp(), order);
    myActiveOrders[order->getId()] = order;
    return event;
}

std::shared_ptr<OrderCancelEvent> OrderEventManagerBase::createOrderCancelEvent(const uint64_t orderId) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end())
        Error::LIB_THROW("OrderEventManagerBase::createOrderCancelEvent: order not found.");
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderCancelEvent>(myEventIdHandler.generateId(), order->getId(), myWorldClock->getCurrentTimestamp());
    myActiveOrders.erase(it);
    return event;
}

std::shared_ptr<OrderModifyPriceEvent> OrderEventManagerBase::createOrderModifyPriceEvent(const uint64_t orderId, const double modifiedPrice) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end())
        Error::LIB_THROW("OrderEventManagerBase::createOrderModifyPriceEvent: order not found.");
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderModifyPriceEvent>(myEventIdHandler.generateId(), order->getId(), myWorldClock->getCurrentTimestamp(), modifiedPrice);
    return event;
}

std::shared_ptr<OrderModifyQuantityEvent> OrderEventManagerBase::createOrderModifyQuantityEvent(const uint64_t orderId, const double modifiedQuantity) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end())
        Error::LIB_THROW("OrderEventManagerBase::createOrderModifyQuantityEvent: order not found.");
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderModifyQuantityEvent>(myEventIdHandler.generateId(), order->getId(), myWorldClock->getCurrentTimestamp(), modifiedQuantity);
    return event;
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::submitLimitOrderEvent(const Side side, const uint32_t quantity, const double price) {
    const auto& event = createLimitOrderSubmitEvent(side, quantity, price);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::submitMarketOrderEvent(const Side side, const uint32_t quantity) {
    const auto& event = createMarketOrderSubmitEvent(side, quantity);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<OrderCancelEvent> OrderEventManagerBase::cancelOrder(const uint64_t orderId) {
    const auto& event = createOrderCancelEvent(orderId);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<OrderModifyPriceEvent> OrderEventManagerBase::modifyOrderPrice(const uint64_t orderId, const double modifiedPrice) {
    const auto& event = createOrderModifyPriceEvent(orderId, modifiedPrice);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<OrderModifyQuantityEvent> OrderEventManagerBase::modifyOrderQuantity(const uint64_t orderId, const double modifiedQuantity) {
    const auto& event = createOrderModifyQuantityEvent(orderId, modifiedQuantity);
    submitOrderEventToMatchingEngine(event);
    return event;
}
}

#endif
