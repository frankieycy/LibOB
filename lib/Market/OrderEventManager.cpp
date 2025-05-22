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
    matchingEngine->setOrderProcessingCallback([this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) { report->dispatchTo(*this); });
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
    return std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), order->getTimestamp(), order);
}

std::shared_ptr<OrderSubmitEvent> OrderEventManagerBase::createMarketOrderSubmitEvent(const Side side, const uint32_t quantity) {
    const auto& order = std::make_shared<MarketOrder>(myOrderIdHandler.generateId(), clockTick(), side, quantity);
    return std::make_shared<OrderSubmitEvent>(myEventIdHandler.generateId(), order->getId(), order->getTimestamp(), order);
}

std::shared_ptr<OrderCancelEvent> OrderEventManagerBase::createOrderCancelEvent(const uint64_t orderId) {
    const auto& order = fetchOrder(orderId);
    if (!order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderCancelEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    return std::make_shared<OrderCancelEvent>(myEventIdHandler.generateId(), order->getId(), clockTick());
}

std::shared_ptr<OrderModifyPriceEvent> OrderEventManagerBase::createOrderModifyPriceEvent(const uint64_t orderId, const double modifiedPrice) {
    const auto& order = fetchOrder(orderId);
    if (!order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderModifyPriceEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    return std::make_shared<OrderModifyPriceEvent>(myEventIdHandler.generateId(), order->getId(), clockTick(), Maths::roundPriceToTick(modifiedPrice, myMinimumPriceTick));
}

std::shared_ptr<OrderModifyQuantityEvent> OrderEventManagerBase::createOrderModifyQuantityEvent(const uint64_t orderId, const double modifiedQuantity) {
    const auto& order = fetchOrder(orderId);
    if (!order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderModifyQuantityEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    return std::make_shared<OrderModifyQuantityEvent>(myEventIdHandler.generateId(), order->getId(), clockTick(), modifiedQuantity);
}

std::shared_ptr<OrderBase> OrderEventManagerBase::fetchOrder(const uint64_t orderId) const {
    const auto& itL = myActiveLimitOrders.find(orderId);
    if (itL == myActiveLimitOrders.end()) {
        const auto& itM = myQueuedMarketOrders.find(orderId);
        if (itM == myQueuedMarketOrders.end()) {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::fetchOrder] Order not found - orderId = " << orderId << ".";
            return nullptr;
        }
        return itM->second;
    }
    return itL->second;
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
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase::onOrderExecutionReport] Order execution report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderExecutionReport] Order execution report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (report.orderExecutionType == Exchange::OrderExecutionType::FILLED || report.orderExecutionType == Exchange::OrderExecutionType::PARTIAL_FILLED) {
        auto order = fetchOrder(report.orderId);
        if (!order) {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderExecutionReport] Order not found in active orders - orderId = " << report.orderId;
            return;
        }
        const uint32_t updatedQuantity = order->getQuantity() - report.filledQuantity;
        order->setQuantity(updatedQuantity);
        if (updatedQuantity == 0) {
            order->setOrderState(Market::OrderState::FILLED);
            if (order->isLimitOrder())
                myActiveLimitOrders.erase(report.orderId);
            else
                myQueuedMarketOrders.erase(report.orderId);
        } else {
            order->setOrderState(Market::OrderState::PARTIAL_FILLED);
        }
    }
}

void OrderEventManagerBase::onOrderSubmitReport(const Exchange::OrderSubmitReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase::onOrderSubmitReport] Order submit report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderSubmitReport] Order submit report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    auto order = report.order->clone();
    if (order->isLimitOrder())
        myActiveLimitOrders[report.orderId] = std::static_pointer_cast<LimitOrder>(order);
    else
        myQueuedMarketOrders[report.orderId] = std::static_pointer_cast<MarketOrder>(order);
}

void OrderEventManagerBase::onOrderCancelReport(const Exchange::OrderCancelReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase::OrderCancelReport] Order cancel report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::OrderCancelReport] Order cancel report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    auto order = fetchOrder(report.orderId);
    if (!order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::OrderCancelReport] Order not found in active orders - orderId = " << report.orderId;
        return;
    }
    order->setOrderState(Market::OrderState::CANCELLED);
    if (order->isLimitOrder())
        myActiveLimitOrders.erase(report.orderId);
    else
        myQueuedMarketOrders.erase(report.orderId);
}

void OrderEventManagerBase::onOrderModifyPriceReport(const Exchange::OrderModifyPriceReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase::onOrderModifyPriceReport] Order modify price report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderModifyPriceReport] Order modify price report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    const auto& it = myActiveLimitOrders.find(report.orderId);
    if (it != myActiveLimitOrders.end()) {
        auto order = it->second;
        order->setPrice(report.modifiedPrice);
    } else {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderModifyPriceReport] Order not found in active orders - orderId = " << report.orderId;
    }
}

void OrderEventManagerBase::onOrderModifyQuantityReport(const Exchange::OrderModifyQuantityReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase::onOrderModifyQuantityReport] Order modify quantity report received.";
    const auto& it = myActiveLimitOrders.find(report.orderId);
    if (it != myActiveLimitOrders.end()) {
        auto order = it->second;
        order->setQuantity(report.modifiedQuantity);
    } else
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderModifyQuantityReport] Order not found in active orders - orderId = " << report.orderId;
}

std::ostream& OrderEventManagerBase::stateSnapshot(std::ostream& out) const {
    out << "============================= Active Orders Snapshot ============================\n";
    out << "   Id   |  Timestamp  |    Type    |   Side   |   Price   |   Size   |   State   \n";
    out << "---------------------------------------------------------------------------------\n";
    for (const auto& orderPair : myActiveLimitOrders) {
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
    for (const auto& orderPair : myQueuedMarketOrders) {
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
