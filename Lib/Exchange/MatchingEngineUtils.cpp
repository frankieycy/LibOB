#ifndef MATCHING_ENGINE_UTILS_CPP
#define MATCHING_ENGINE_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/ITCHEncoder.hpp"
#include "Parser/LobsterDataParser.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

namespace Exchange {
using namespace Utils;

std::string toString(const OrderMatchingStrategy& orderMatchingStrategy) {
    switch (orderMatchingStrategy) {
        case OrderMatchingStrategy::FIFO:            return "FIFO";
        case OrderMatchingStrategy::PRO_RATA:        return "ProRata";
        case OrderMatchingStrategy::ICEBERG_SUPPORT: return "IcebergSupport";
        default:                                     return "None";
    }
}

std::string toString(const OrderProcessingType& orderProcessingType) {
    switch (orderProcessingType) {
        case OrderProcessingType::EXECUTE:           return "Execution";
        case OrderProcessingType::SUBMIT:            return "Submission";
        case OrderProcessingType::PLACEMENT:         return "Placement";
        case OrderProcessingType::CANCEL:            return "Cancel";
        case OrderProcessingType::PARTIAL_CANCEL:    return "PartialCancel";
        case OrderProcessingType::CANCEL_REPLACE:    return "CancelReplace";
        case OrderProcessingType::MODIFY_PRICE:      return "ModifyPrice";
        case OrderProcessingType::MODIFY_QUANTITY:   return "ModifyQuantity";
        default:                                     return "None";
    }
}

std::string toString(const OrderProcessingStatus& orderProcessingStatus) {
    switch (orderProcessingStatus) {
        case OrderProcessingStatus::SUCCESS:         return "Success";
        case OrderProcessingStatus::FAILURE:         return "Failure";
        default:                                     return "None";
    }
}

std::string toString(const OrderExecutionType& orderExecutionType) {
    switch (orderExecutionType) {
        case OrderExecutionType::FILLED:             return "Filled";
        case OrderExecutionType::PARTIAL_FILLED:     return "PartialFilled";
        case OrderExecutionType::CANCELLED:          return "Cancelled";
        case OrderExecutionType::REJECTED:           return "Rejected";
        default:                                     return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy) { return out << toString(orderMatchingStrategy); }

std::ostream& operator<<(std::ostream& out, const OrderProcessingType& orderProcessingType) { return out << toString(orderProcessingType); }

std::ostream& operator<<(std::ostream& out, const OrderProcessingStatus& orderProcessingStatus) { return out << toString(orderProcessingStatus); }

std::ostream& operator<<(std::ostream& out, const OrderExecutionType& orderExecutionType) { return out << toString(orderExecutionType); }

std::ostream& operator<<(std::ostream& out, const OrderProcessingReport& report) { return out << report.getAsJson(); }

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

void OrderExecutionReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderExecutionReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

/* We do NOT infer order submit event from execution report since it has already
    been accounted for in submit report, otherwise there are duplicate orders. */
std::shared_ptr<Market::OrderEventBase> OrderExecutionReport::makeEvent() const {
    // std::shared_ptr<Market::OrderBase> order;
    // if (orderType == Market::OrderType::LIMIT)
    //     order = std::make_shared<Market::LimitOrder>(orderId, timestamp, orderSide, filledQuantity, filledPrice);
    // else if (orderType == Market::OrderType::MARKET)
    //     order = std::make_shared<Market::MarketOrder>(orderId, timestamp, orderSide, filledQuantity);
    // if (order && !isMakerOrder) // maker orders passively rest on the book; only taker orders imply an order submit event
    //     return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order);
    return nullptr;
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderExecutionReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderExecutionReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderExecutionReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderType\":\""           << orderType            << "\","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"MatchOrderId\":"          << matchOrderId         << ","
        "\"TradeId\":"               << tradeId              << ","
        "\"FilledQuantity\":"        << filledQuantity       << ","
        "\"FilledPrice\":"           << filledPrice          << ","
        "\"IsMakerOrder\":"          << isMakerOrder         << ","
        "\"OrderExecutionType\":\""  << orderExecutionType   << "\","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void LimitOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void LimitOrderSubmitReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void LimitOrderSubmitReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> LimitOrderSubmitReport::makeEvent() const {
    return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order->clone());
}

std::shared_ptr<ITCHEncoder::ITCHMessage> LimitOrderSubmitReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> LimitOrderSubmitReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
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
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void LimitOrderPlacementReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void LimitOrderPlacementReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> LimitOrderPlacementReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string LimitOrderPlacementReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderQuantity\":"         << orderQuantity        << ","
        "\"OrderPrice\":"            << orderPrice           << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void MarketOrderSubmitReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void MarketOrderSubmitReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void MarketOrderSubmitReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> MarketOrderSubmitReport::makeEvent() const {
    return std::make_shared<Market::OrderSubmitEvent>(reportId, orderId, timestamp, order->clone());
}

std::shared_ptr<ITCHEncoder::ITCHMessage> MarketOrderSubmitReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> MarketOrderSubmitReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
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
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderCancelReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void OrderCancelReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderCancelReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderCancelReport::makeEvent() const {
    return std::make_shared<Market::OrderCancelEvent>(reportId, orderId, timestamp);
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderCancelReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderCancelReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderCancelReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderType\":\""           << orderType            << "\","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderPartialCancelReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void OrderPartialCancelReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderPartialCancelReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::shared_ptr<Market::OrderEventBase> OrderPartialCancelReport::makeEvent() const {
    return std::make_shared<Market::OrderPartialCancelEvent>(reportId, orderId, timestamp, cancelQuantity);
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderPartialCancelReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderPartialCancelReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderPartialCancelReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderType\":\""           << orderType            << "\","
        "\"CancelQuantity\":"        << cancelQuantity       << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderCancelAndReplaceReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void OrderCancelAndReplaceReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderCancelAndReplaceReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::vector<std::shared_ptr<const OrderProcessingReport>> OrderCancelAndReplaceReport::decomposeIntoAtomicReports() const {
    return {
        std::make_shared<OrderCancelReport>(reportId, timestamp, orderId, orderSide, Market::OrderType::LIMIT, std::nullopt, std::nullopt, status),
        std::make_shared<LimitOrderSubmitReport>(reportId, timestamp, orderId, orderSide,
            std::make_shared<Market::LimitOrder>(newOrderId, timestamp, orderSide, newQuantity, newPrice),
            status)
    };
}

std::shared_ptr<Market::OrderEventBase> OrderCancelAndReplaceReport::makeEvent() const {
    return std::make_shared<Market::OrderCancelAndReplaceEvent>(reportId, orderId, timestamp, newOrderId, newQuantity, newPrice);
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderCancelAndReplaceReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderCancelAndReplaceReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderCancelAndReplaceReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderType\":\""           << orderType            << "\","
        "\"NewOrderId\":"            << newOrderId           << ","
        "\"NewQuantity\":"           << newQuantity          << ","
        "\"NewPrice\":"              << newPrice             << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyPriceReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void OrderModifyPriceReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderModifyPriceReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::vector<std::shared_ptr<const OrderProcessingReport>> OrderModifyPriceReport::decomposeIntoAtomicReports() const {
    return {
        std::make_shared<OrderCancelReport>(reportId, timestamp, orderId, orderSide, Market::OrderType::LIMIT, std::nullopt, std::nullopt, status),
        std::make_shared<LimitOrderSubmitReport>(reportId, timestamp, orderId, orderSide,
            std::make_shared<Market::LimitOrder>(orderId, timestamp, orderSide, orderQuantity, modifiedPrice),
            status)
    };
}

std::shared_ptr<Market::OrderEventBase> OrderModifyPriceReport::makeEvent() const {
    return std::make_shared<Market::OrderModifyPriceEvent>(reportId, orderId, timestamp, modifiedPrice);
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderModifyPriceReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderModifyPriceReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderModifyPriceReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderQuantity\":"         << orderQuantity        << ","
        "\"ModifiedPrice\":"         << modifiedPrice        << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}

void OrderModifyQuantityReport::dispatchTo(Market::OrderEventManagerBase& orderEventManager) const {
    orderEventManager.onOrderProcessingReport(*this);
}

void OrderModifyQuantityReport::dispatchTo(Analytics::MatchingEngineMonitor& matchingEngineMonitor) const {
    matchingEngineMonitor.onOrderProcessingReport(*this);
}

void OrderModifyQuantityReport::dispatchTo(Analytics::MonitorOutputsAnalyzerBase& monitorOutputsAnalyzer) const {
    monitorOutputsAnalyzer.onOrderProcessingReport(*this);
}

std::vector<std::shared_ptr<const OrderProcessingReport>> OrderModifyQuantityReport::decomposeIntoAtomicReports() const {
    return {
        std::make_shared<OrderCancelReport>(reportId, timestamp, orderId, orderSide, Market::OrderType::LIMIT, std::nullopt, std::nullopt, status),
        std::make_shared<LimitOrderSubmitReport>(reportId, timestamp, orderId, orderSide,
            std::make_shared<Market::LimitOrder>(orderId, timestamp, orderSide, modifiedQuantity, orderPrice),
            status)
    };
}

std::shared_ptr<Market::OrderEventBase> OrderModifyQuantityReport::makeEvent() const {
    return std::make_shared<Market::OrderModifyQuantityEvent>(reportId, orderId, timestamp, modifiedQuantity);
}

std::shared_ptr<ITCHEncoder::ITCHMessage> OrderModifyQuantityReport::makeITCHMessage() const {
    return ITCHEncoder::encodeReport(*this);
}

std::shared_ptr<Parser::LobsterDataParser::OrderBookMessage> OrderModifyQuantityReport::makeLobsterMessage() const {
    return std::make_shared<Parser::LobsterDataParser::OrderBookMessage>(*this);
}

std::string OrderModifyQuantityReport::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
        "\"ReportId\":"              << reportId             << ","
        "\"Timestamp\":"             << timestamp            << ","
        "\"OrderId\":"               << orderId              << ","
        "\"OrderSide\":\""           << orderSide            << "\","
        "\"OrderPrice\":"            << orderPrice           << ","
        "\"ModifiedQuantity\":"      << modifiedQuantity     << ","
        "\"OrderProcessingType\":\"" << orderProcessingType  << "\","
        "\"Status\":\""              << status               << "\","
        "\"AgentIdHash\":"           << agentIdHash.value_or(0) << ","
        "\"Latency\":"               << latency.value_or(0)  << ","
        "\"Message\":\""             << message.value_or("") << "\"";
    oss << "}";
    return oss.str();
}
}

#endif
