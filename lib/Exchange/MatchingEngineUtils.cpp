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

std::string generateBar(uint32_t size, uint32_t maxSize, int maxWidth) {
    int barWidth = (maxSize == 0) ? 0 : static_cast<int>((static_cast<double>(size) / maxSize) * maxWidth);
    return std::string(barWidth, 'o');
}

std::string getOrderBookASCII(
    const std::vector<OrderLevel>& bidBook,
    const std::vector<OrderLevel>& askBook,
    const int barWidth,
    const size_t maxDepth) {
        std::ostringstream oss;
        const int fieldWidth = 17;
        const int sideWidth = barWidth + 1 + fieldWidth;

        oss << std::string(2 * sideWidth + 11, '=') << '\n';
        oss << std::setw(sideWidth) << std::right << "(Size | Price) BID"
            << " | Level | "
            << std::setw(sideWidth) << std::left << "ASK (Price | Size)"
            << '\n';
        oss << std::string(2 * sideWidth + 11, '-') << '\n';

        size_t depth = std::min(std::max(bidBook.size(), askBook.size()), maxDepth);
        uint32_t maxSize = 0;
        for (const auto& b : bidBook) maxSize = std::max(maxSize, b.size);
        for (const auto& a : askBook) maxSize = std::max(maxSize, a.size);

        for (size_t i = 0; i < depth; ++i) {
            std::ostringstream bidStream, askStream;

            if (i < bidBook.size()) {
                const auto& b = bidBook[i];
                std::string bar = generateBar(b.size, maxSize, barWidth);
                std::ostringstream label;
                label << std::setw(6) << b.size << " | "
                      << std::fixed << std::setprecision(2) << std::setw(8) << b.price;
                bidStream << std::right << std::setw(barWidth) << std::right << bar << " " << label.str();
            } else {
                bidStream << std::string(sideWidth, ' ');
            }

            if (i < askBook.size()) {
                const auto& a = askBook[i];
                std::string bar = generateBar(a.size, maxSize, barWidth);
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
    }
}

void OrderExecutionReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::string OrderExecutionReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"TradeId\":"               << tradeId              << ","
        "\"FilledQuantity\":"        << filledQuantity       << ","
        "\"FilledPrice\":"           << filledPrice          << ","
        "\"IsMakerOrder\":"          << isMakerOrder         << ","
        "\"OrderExecutionType\":\""  << orderExecutionType   << "\","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void LimitOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::string LimitOrderSubmitReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Order\":"                 << order                << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void MarketOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

std::string MarketOrderSubmitReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"Order\":"                 << order                << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderCancelReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
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
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyPriceReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
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
        "\"Latency\":"               << latency.value_or(Consts::quietNaN<uint64_t>()) << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyQuantityReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
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
