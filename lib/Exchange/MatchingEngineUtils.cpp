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

std::ostream& operator<<(std::ostream& out, const OrderProcessingReport& event) { return out << event.getAsJson(); }

std::string generateBar(const uint32_t size, const uint32_t maxSize, const size_t maxWidth, const char symbol) {
    const size_t barWidth = (maxSize == 0) ? 0 : static_cast<size_t>((static_cast<double>(size) / maxSize) * maxWidth);
    return std::string(barWidth, symbol);
}

std::string getOrderBookASCII(
    const std::vector<OrderLevel>& bidBook,
    const std::vector<OrderLevel>& askBook,
    const size_t barWidth,
    const size_t maxDepth) {
        std::ostringstream oss;
        const size_t fieldWidth = 17;
        const size_t sideWidth = barWidth + 1 + fieldWidth;

        oss << std::string(2 * sideWidth + 11, '=') << '\n';
        oss << std::setw(sideWidth) << std::right << "(Size | Price) BID"
            << " | Level | "
            << std::setw(sideWidth) << std::left << "ASK (Price | Size)"
            << '\n';
        oss << std::string(2 * sideWidth + 11, '-') << '\n';

        const size_t depth = std::min(std::max(bidBook.size(), askBook.size()), maxDepth);
        uint32_t maxSize = 0;
        for (const auto& b : bidBook) maxSize = std::max(maxSize, b.size);
        for (const auto& a : askBook) maxSize = std::max(maxSize, a.size);

        for (size_t i = 0; i < depth; ++i) {
            std::ostringstream bidStream, askStream;

            if (i < bidBook.size()) {
                const auto& b = bidBook[i];
                const std::string& bar = generateBar(b.size, maxSize, barWidth);
                std::ostringstream label;
                label << std::setw(6) << b.size << " | "
                      << std::fixed << std::setprecision(2) << std::setw(8) << b.price;
                bidStream << std::right << std::setw(barWidth) << std::right << bar << " " << label.str();
            } else {
                bidStream << std::string(sideWidth, ' ');
            }

            if (i < askBook.size()) {
                const auto& a = askBook[i];
                const std::string& bar = generateBar(a.size, maxSize, barWidth);
                std::ostringstream label;
                label << std::left << std::fixed << std::setprecision(2) << std::setw(8) << a.price
                      << " | " << std::setw(6) << a.size;
                askStream << label.str() << " " << bar;
            } else {
                askStream << std::string(sideWidth, ' ');
            }

            oss << std::setw(sideWidth) << std::right << bidStream.str()
                << " | " << std::setw(5) << std::right << i + 1 << " | "
                << std::setw(sideWidth) << std::left << askStream.str()
                << '\n';
        }

        oss << std::string(2 * sideWidth + 11, '=') << '\n';
        return oss.str();
}

OrderBookDisplayConfig::OrderBookDisplayConfig(const bool debugMode) : OrderBookDisplayConfig() {
    setDebugMode(debugMode);
}

void OrderBookDisplayConfig::setDebugMode(const bool debugMode) {
    myDebugMode = debugMode;
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
    } else {
        myOrderBookLevels = 5;
        myMarketQueueLevels = 10;
        myTradeLogLevels = 10;
        myRemovedLimitOrderLogLevels = 10;
        myOrderLookupLevels = 10;
        myAggregateOrderBook = true;
        myShowOrderBook = true;
        myShowMarketQueue = true;
        myShowTradeLog = true;
        myShowRemovedLimitOrderLog = true;
        myShowOrderLookup = true;
    }
}

void OrderExecutionReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderExecutionReport::makeEvent() const {
    std::shared_ptr<Market::OrderBase> order;
    if (orderType == Market::OrderType::LIMIT)
        order = std::make_shared<Market::LimitOrder>(orderId, timestamp, orderSide, filledQuantity, filledPrice);
    else if (orderType == Market::OrderType::MARKET)
        std::make_shared<Market::MarketOrder>(orderId, timestamp, orderSide, filledQuantity);
    if (order && !isMakerOrder) // maker orders passively rest on the book; only taker orders imply an order submit event
        return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order);
    return nullptr;
}

std::string OrderExecutionReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderType\":\""           << orderType            << "\","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"TradeId\":"               << tradeId              << ","
        "\"FilledQuantity\":"        << filledQuantity       << ","
        "\"FilledPrice\":"           << filledPrice          << ","
        "\"IsMakerOrder\":"          << isMakerOrder         << ","
        "\"OrderExecutionType\":\""  << orderExecutionType   << "\","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void LimitOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> LimitOrderSubmitReport::makeEvent() const {
    return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order->clone());
}

std::string LimitOrderSubmitReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Order\":"                 << *order               << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void MarketOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> MarketOrderSubmitReport::makeEvent() const {
    return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order->clone());
}

std::string MarketOrderSubmitReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Order\":"                 << *order               << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderCancelReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderCancelReport::makeEvent() const {
    return std::make_shared<Market::OrderCancelEvent>(reportId, orderId, timestamp);
}

std::string OrderCancelReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyPriceReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderModifyPriceReport::makeEvent() const {
    return std::make_shared<Market::OrderModifyPriceEvent>(reportId, orderId, timestamp, modifiedPrice);
}

std::string OrderModifyPriceReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"ModifiedPrice\":"         << modifiedPrice        << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyQuantityReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderModifyQuantityReport::makeEvent() const {
    return std::make_shared<Market::OrderModifyQuantityEvent>(reportId, orderId, timestamp, modifiedQuantity);
}

std::string OrderModifyQuantityReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"ModifiedQuantity\":"      << modifiedQuantity     << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}
}

#endif
