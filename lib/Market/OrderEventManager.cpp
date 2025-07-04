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
    return manager.stateSnapshot(out);
}

OrderEventManagerBase::OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderEventManagerBase] Matching engine is null.");
    mySyncClockWithEngine = true;
    myMatchingEngine = matchingEngine;
    myWorldClock = matchingEngine->getWorldClock();
    myDebugMode = matchingEngine->isDebugMode();
    myOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
}

void OrderEventManagerBase::submitOrderEventToMatchingEngine(const std::shared_ptr<OrderEventBase>& event) {
    if (!event) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::submitOrderEventToMatchingEngine] Order event is null - omitting submission.";
        return;
    }
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order event submitted: " << *event;
    if (myMillisecondsToPauseBeforeEventSubmit > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(myMillisecondsToPauseBeforeEventSubmit));
    if (myTimeEngineOrderEventsProcessing) {
        const auto& duration = Counter::timeOperation<std::chrono::microseconds>([this, event]() { myMatchingEngine->process(event); }); // typical timescale of NASDAQ engine is in milliseconds
        *myLogger << Logger::LogLevel::INFO << "[OrderEventManagerBase] Matching engine order event processing time: " << duration << " microseconds. Note that IO operations are also counted in.";
    } else {
        myMatchingEngine->process(event);
    }
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

std::shared_ptr<OrderCancelAndReplaceEvent> OrderEventManagerBase::createOrderCancelAndReplaceEvent(const uint64_t orderId,
    const std::optional<uint32_t>& modifiedQuantity, const std::optional<double>& modifiedPrice) {
    const auto& order = fetchOrder(orderId);
    if (!order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::createOrderCancelAndReplaceEvent] Order not found - orderId = " << orderId << ".";
        return nullptr;
    }
    const auto& roundedModifiedPrice = modifiedPrice ? std::make_optional(Maths::roundPriceToTick(*modifiedPrice, myMinimumPriceTick)) : std::nullopt;
    return std::make_shared<OrderCancelAndReplaceEvent>(myEventIdHandler.generateId(), order->getId(), clockTick(), myOrderIdHandler.generateId(), modifiedQuantity, roundedModifiedPrice);
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

LimitOrderIndex::const_iterator OrderEventManagerBase::fetchLimitOrderIterator(const uint64_t orderId) const {
    const auto& it = myActiveLimitOrders.find(orderId);
    if (it == myActiveLimitOrders.end()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::fetchLimitOrderIterator] Limit order not found - orderId = " << orderId << ".";
        return myActiveLimitOrders.cend();
    }
    return it;
}

MarketOrderIndex::const_iterator OrderEventManagerBase::fetchMarketOrderIterator(const uint64_t orderId) const {
    const auto& it = myQueuedMarketOrders.find(orderId);
    if (it == myQueuedMarketOrders.end()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::fetchMarketOrderIterator] Market order not found - orderId = " << orderId << ".";
        return myQueuedMarketOrders.cend();
    }
    return it;
}

void OrderEventManagerBase::setLoggerLogFile(const std::string& logFileName, const bool logToConsole, const bool showLogTimestamp) {
    if (logFileName.empty()) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::setLoggerLogFile] Log file name is empty, logger will not log to file.";
        return;
    }
    myLogger->setLogFile(logFileName, logToConsole, showLogTimestamp);
    myMatchingEngine->setLogger(myLogger);
    *myLogger << Logger::LogLevel::INFO << "[OrderEventManagerBase::setLoggerLogFile] Logger log file set to: " << logFileName;
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

std::shared_ptr<const OrderCancelAndReplaceEvent> OrderEventManagerBase::cancelAndReplaceOrder(const uint64_t orderId,
    const std::optional<uint32_t>& modifiedQuantity, const std::optional<double>& modifiedPrice) {
    const auto& event = createOrderCancelAndReplaceEvent(orderId, modifiedQuantity, modifiedPrice);
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

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::OrderExecutionReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order execution report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order execution report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (report.orderExecutionType == Exchange::OrderExecutionType::FILLED || report.orderExecutionType == Exchange::OrderExecutionType::PARTIAL_FILLED) {
        auto order = fetchOrder(report.orderId);
        if (!order) {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order not found in active orders - orderId = " << report.orderId;
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

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order submit report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order submit report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (!report.order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order is null - orderId = " << report.orderId;
        return;
    }
    auto order = report.order->copy(); // keep an internal clone of the order
    order->setTimestamp(clockTick());
    if (order->isAlive())
        myActiveLimitOrders[order->getId()] = order;
}

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order submit report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order submit report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (!report.order) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order is null - orderId = " << report.orderId;
        return;
    }
    auto order = report.order->copy(); // keep an internal clone of the order
    order->setTimestamp(clockTick());
    if (order->isAlive())
        myQueuedMarketOrders[order->getId()] = order;
}

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::OrderCancelReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order cancel report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order cancel report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (report.orderType == Market::OrderType::LIMIT) {
        const auto& it = fetchLimitOrderIterator(report.orderId);
        if (it != myActiveLimitOrders.end()) {
            auto order = it->second;
            order->setOrderState(Market::OrderState::CANCELLED);
            order->setTimestamp(clockTick());
            myActiveLimitOrders.erase(it);
        } else {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Limit order not found in active limit orders - orderId = " << report.orderId;
        }
    } else if (report.orderType == Market::OrderType::MARKET) {
        const auto& it = fetchMarketOrderIterator(report.orderId);
        if (it != myQueuedMarketOrders.end()) {
            auto order = it->second;
            order->setOrderState(Market::OrderState::CANCELLED);
            order->setTimestamp(clockTick());
            myQueuedMarketOrders.erase(it);
        } else {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Market order not found in queued market orders - orderId = " << report.orderId;
        }
    } else {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Unknown order type in cancel report - orderId = " << report.orderId;
    }
}

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order cancel and replace report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order cancel and replace report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    if (report.orderType == Market::OrderType::LIMIT) {
        const auto& it = fetchLimitOrderIterator(report.orderId);
        if (it != myActiveLimitOrders.end()) {
            auto order = it->second;
            myActiveLimitOrders.erase(it); // remove the old order
            order->setId(report.newOrderId);
            order->setQuantity(report.newQuantity);
            order->setPrice(report.newPrice);
            order->setTimestamp(clockTick());
            if (order->isAlive()) {
                order->setOrderState(Market::OrderState::ACTIVE);
                myActiveLimitOrders[order->getId()] = order; // reinsert the updated order
            }
        } else {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Limit order not found in active limit orders - orderId = " << report.orderId;
        }
    } else if (report.orderType == Market::OrderType::MARKET) {
        const auto& it = fetchMarketOrderIterator(report.orderId);
        if (it != myQueuedMarketOrders.end()) {
            auto order = it->second;
            myQueuedMarketOrders.erase(it); // remove the old order
            order->setId(report.newOrderId);
            order->setQuantity(report.newQuantity);
            order->setTimestamp(clockTick());
            if (order->isAlive()) {
                order->setOrderState(Market::OrderState::ACTIVE);
                myQueuedMarketOrders[order->getId()] = order; // reinsert the updated order
            }
        } else {
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Market order not found in queued market orders - orderId = " << report.orderId;
        }
    } else {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Unknown order type in cancel and replace report - orderId = " << report.orderId;
    }
}

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order modify price report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order modify price report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    const auto& it = myActiveLimitOrders.find(report.orderId);
    if (it != myActiveLimitOrders.end()) {
        auto order = it->second;
        order->setPrice(report.modifiedPrice);
        order->setTimestamp(clockTick());
        if (!order->checkState())
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order in an invalid state upon price modification: " << *order;
    } else {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order not found in active orders - orderId = " << report.orderId;
    }
}

void OrderEventManagerBase::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report) {
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[OrderEventManagerBase] Order modify quantity report received: " << report;
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS) {
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order modify quantity report status is NOT success, skipping active orders update - orderId = " << report.orderId;
        return;
    }
    const auto& it = myActiveLimitOrders.find(report.orderId);
    if (it != myActiveLimitOrders.end()) {
        auto order = it->second;
        order->setQuantity(report.modifiedQuantity);
        order->setTimestamp(clockTick());
        if (!order->checkState())
            *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order in an invalid state upon quantity modification: " << *order;
    } else
        *myLogger << Logger::LogLevel::WARNING << "[OrderEventManagerBase::onOrderProcessingReport] Order not found in active orders - orderId = " << report.orderId;
}

std::ostream& OrderEventManagerBase::stateSnapshot(std::ostream& out) const {
    out << "============================== Active Orders Snapshot =============================\n";
    out << "    Id    |  Timestamp  |    Type    |   Side   |   Price   |   Size   |   State   \n";
    out << "-----------------------------------------------------------------------------------\n";
    for (const auto& orderPair : myActiveLimitOrders) {
        const auto& order = orderPair.second;
        out << std::setw(8) << order->getId() << "  | "
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
        out << std::setw(8) << order->getId() << "  | "
            << std::setw(10) << order->getTimestamp() << "  | "
            << std::setw(9) << order->getOrderType() << "  | "
            << std::setw(7) << order->getSide() << "  | "
            << std::fixed << std::setprecision(2)
            << std::setw(8) << order->getPrice() << "  | "
            << std::setw(7) << order->getQuantity() << "  | "
            << std::setw(8) << order->getOrderState() << "  \n";
    }
    out << "-----------------------------------------------------------------------------------\n";
    return out;
}

void OrderEventManagerBase::reserve(const size_t numOrdersEstimate) {
    myMatchingEngine->reserve(numOrdersEstimate);
    myActiveLimitOrders.reserve(numOrdersEstimate);
    myQueuedMarketOrders.reserve(numOrdersEstimate);
}
}

#endif
