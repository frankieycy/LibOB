#ifndef MATCHING_ENGINE_UTILS_CPP
#define MATCHING_ENGINE_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

namespace Exchange {
using namespace Utils;

std::string to_string(const OrderMatchingStrategy& orderMatchingStrategy) {
    switch (orderMatchingStrategy) {
        case OrderMatchingStrategy::FIFO:            return "FIFO";
        case OrderMatchingStrategy::PRO_RATA:        return "ProRata";
        case OrderMatchingStrategy::ICEBERG_SUPPORT: return "IcebergSupport";
        default:                                     return "Null";
    }
}

std::string to_string(const OrderProcessingType& orderProcessingType) {
    switch (orderProcessingType) {
        case OrderProcessingType::EXECUTE:           return "Execution";
        case OrderProcessingType::SUBMIT:            return "Submission";
        case OrderProcessingType::CANCEL:            return "Cancel";
        case OrderProcessingType::MODIFY_PRICE:      return "ModifyPrice";
        case OrderProcessingType::MODIFY_QUANTITY:   return "ModifyQuantity";
        default:                                     return "Null";
    }
}

std::string to_string(const OrderProcessingStatus& orderProcessingStatus) {
    switch (orderProcessingStatus) {
        case OrderProcessingStatus::SUCCESS:         return "Success";
        case OrderProcessingStatus::FAILURE:         return "Failure";
        default:                                     return "Null";
    }
}

std::string to_string(const OrderExecutionType& orderExecutionType) {
    switch (orderExecutionType) {
        case OrderExecutionType::FILLED:             return "Filled";
        case OrderExecutionType::PARTIAL_FILLED:     return "PartialFilled";
        case OrderExecutionType::CANCELLED:          return "Cancelled";
        case OrderExecutionType::REJECTED:           return "Rejected";
        default:                                     return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy) { return out << to_string(orderMatchingStrategy); }

std::ostream& operator<<(std::ostream& out, const OrderProcessingType& orderProcessingType) { return out << to_string(orderProcessingType); }

std::ostream& operator<<(std::ostream& out, const OrderProcessingStatus& orderProcessingStatus) { return out << to_string(orderProcessingStatus); }

std::ostream& operator<<(std::ostream& out, const OrderExecutionType& orderExecutionType) { return out << to_string(orderExecutionType); }

OrderBookDisplayConfig::OrderBookDisplayConfig(const bool debugMode) :
    myDebugMode(debugMode) {
    if (myDebugMode) {
        myOrderBookLevels = 20;
        myMarketQueueLevels = 20;
        myTradeLogLevels = 20;
        myRemovedLimitOrderLogLevels = 20;
        myOrderLookupLevels = 20;
        myAggregateOrderBook = false;
        myShowOrderBook = true;
        myShowMarketQueue = true;
        myShowTradeLog = true;
        myShowRemovedLimitOrderLog = true;
        myShowOrderLookup = true;
    }
}

void OrderExecutionReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderExecutionReport(*this);
}

std::string OrderExecutionReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"TradeId\":"               << tradeId              << ","
        "\"FilledQuantity\":"        << filledQuantity       << ","
        "\"FilledPrice\":"           << filledPrice          << ","
        "\"IsMakerOrder\":"          << isMakerOrder         << ","
        "\"OrderExecutionType\":\""  << orderExecutionType   << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderSubmitReport(*this);
}

std::string OrderCancelReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderCancelReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderCancelReport(*this);
}

std::string OrderModifyPriceReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"ModifiedPrice\":"         << modifiedPrice        << ","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyPriceReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderModifyPriceReport(*this);
}

std::string OrderModifyQuantityReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"ModifiedQuantity\":"       << modifiedQuantity     << ","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyQuantityReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderModifyQuantityReport(*this);
}

std::string OrderProcessingReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& out, const OrderProcessingReport& event) {
    out << event.getAsJson();
    return out;
}
}

#endif
