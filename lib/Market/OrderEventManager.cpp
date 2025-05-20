#ifndef ORDER_EVENT_MANAGER_CPP
#define ORDER_EVENT_MANAGER_CPP
#include "Utils/Utils.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEvent.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Market {
using namespace Utils;
std::ostream& operator<<(std::ostream& out, const OrderEventManagerBase& manager) {
    manager.stateSnapshot(out);
    return out;
}

OrderEventManagerBase::OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderEventManagerBase] Matching engine is null.");
    mySyncClockWithEngine = true;
    myMatchingEngine = matchingEngine;
    myWorldClock = matchingEngine->getWorldClock();
    myDebugMode = matchingEngine->isDebugMode();
    matchingEngine->setOrderExecutionCallback([this](const Exchange::OrderExecutionReport& report) { onOrderExecutionReport(report); });
}

void OrderEventManagerBase::submitOrderEventToMatchingEngine(const std::shared_ptr<OrderEventBase>& event) {
    if (!event) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::submitOrderEventToMatchingEngine] Order event is null - omitting submission.";
        return;
    }
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order event submitted: " << *event;
    myMatchingEngine->process(event);
    if (myDebugMode) {
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order event manager state:\n" << *this;
        if (myPrintOrderBookPerOrderSubmit)
            *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order book state:\n" << *myMatchingEngine;
    }
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::createLimitOrderSubmitEvent(const Side side, const uint32_t quantity, const double price) {
    const auto& order = std::make_shared<LimitOrder>(myOrderIdHandler.generateId(), clockTick(), side, quantity, Maths::roundPriceToTick(price, myMinimumPriceTick));
    const auto& event = std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), order->getTimestamp(), order);
    myActiveOrders[order->getId()] = order;
    return event;
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::createMarketOrderSubmitEvent(const Side side, const uint32_t quantity) {
    const auto& order = std::make_shared<MarketOrder>(myOrderIdHandler.generateId(), clockTick(), side, quantity);
    const auto& event = std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), order->getTimestamp(), order);
    myActiveOrders[order->getId()] = order;
    return event;
}

std::shared_ptr<OrderCancelEvent> OrderEventManagerBase::createOrderCancelEvent(const uint64_t orderId) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderCancelEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderCancelEvent>(myEventIdHandler.generateId(), order->getId(), clockTick());
    myActiveOrders.erase(it);
    return event;
}

std::shared_ptr<OrderModifyPriceEvent> OrderEventManagerBase::createOrderModifyPriceEvent(const uint64_t orderId, const double modifiedPrice) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderModifyPriceEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderModifyPriceEvent>(myEventIdHandler.generateId(), order->getId(), clockTick(), modifiedPrice);
    return event;
}

std::shared_ptr<OrderModifyQuantityEvent> OrderEventManagerBase::createOrderModifyQuantityEvent(const uint64_t orderId, const double modifiedQuantity) {
    const auto& it = myActiveOrders.find(orderId);
    if (it == myActiveOrders.end()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderModifyQuantityEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    const auto& order = it->second;
    const auto& event = std::make_shared<OrderModifyQuantityEvent>(myEventIdHandler.generateId(), order->getId(), clockTick(), modifiedQuantity);
    return event;
}

std::shared_ptr<const OrderSubmitEvent> OrderEventManagerBase::submitLimitOrderEvent(const Side side, const uint32_t quantity, const double price) {
    const auto& event = createLimitOrderSubmitEvent(side, quantity, price);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<const OrderSubmitEvent> OrderEventManagerBase::submitMarketOrderEvent(const Side side, const uint32_t quantity) {
    const auto& event = createMarketOrderSubmitEvent(side, quantity);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<const OrderCancelEvent> OrderEventManagerBase::cancelOrder(const uint64_t orderId) {
    const auto& event = createOrderCancelEvent(orderId);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<const OrderModifyPriceEvent> OrderEventManagerBase::modifyOrderPrice(const uint64_t orderId, const double modifiedPrice) {
    const auto& event = createOrderModifyPriceEvent(orderId, modifiedPrice);
    submitOrderEventToMatchingEngine(event);
    return event;
}

std::shared_ptr<const OrderModifyQuantityEvent> OrderEventManagerBase::modifyOrderQuantity(const uint64_t orderId, const double modifiedQuantity) {
    const auto& event = createOrderModifyQuantityEvent(orderId, modifiedQuantity);
    submitOrderEventToMatchingEngine(event);
    return event;
}

void OrderEventManagerBase::onOrderExecutionReport(const Exchange::OrderExecutionReport& report) {
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

void OrderEventManagerBase::onOrderSubmitReport(const Exchange::OrderSubmitReport& /* report */) {}

void OrderEventManagerBase::onOrderCancelReport(const Exchange::OrderCancelReport& /* report */) {}

void OrderEventManagerBase::onOrderModifyPriceReport(const Exchange::OrderModifyPriceReport& /* report */) {}

void OrderEventManagerBase::onOrderModifyQuantityReport(const Exchange::OrderModifyQuantityReport& /* report */) {}


std::ostream& OrderEventManagerBase::stateSnapshot(std::ostream& out) const {
    out << "============================= Active Orders Snapshot ============================\n";
    out << "   Id   |  Timestamp  |    Type    |   Side   |   Price   |   Size   |   State   \n";
    out << "---------------------------------------------------------------------------------\n";
    for (const auto& orderPair : myActiveOrders) {
        const auto& order = orderPair.second;
        out << std::setw(6) << order->getId() << "  | "
            << std::setw(10) << order->getTimestamp() << "  | "
            << std::setw(9) << order->getOrderType() << "  | "
            << std::setw(7) << order->getSide() << "  | "
            << std::fixed << std::setprecision(2)
            << std::setw(8) << order->getPrice() << "  | "
            << std::setw(7) << order->getQuantity() << "  | "
            << std::setw(8) << order->getOrderState() << "  \n";
    }
    out << "---------------------------------------------------------------------------------\n";
    return out;
}
}

#endif
