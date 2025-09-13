#ifndef LOBSTER_DATA_PARSER_CPP
#define LOBSTER_DATA_PARSER_CPP
#include "LobsterDataParser.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

namespace Parser {
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderExecutionReport& report) {
    // assume only visible executions for now
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return;
    if (report.orderType == Market::OrderType::LIMIT) {
        timestamp = report.timestamp;
        messageType = MessageType::ORDER_EXECUTE_VISIBLE;
        orderId = report.orderId;
        quantity = report.filledQuantity;
        price = Maths::castDoublePriceAsInt<uint32_t>(report.filledPrice);
        isBuy = report.orderSide == Market::Side::BUY;
    }
}

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::LimitOrderSubmitReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return;
    const auto& order = report.order;
    if (!order)
        Error::LIB_THROW("ITCHEncoder::encodeReport: LimitOrderSubmitReport order is null");
    timestamp = report.timestamp;
    messageType = MessageType::ORDER_ADD;
    orderId = order->getId();
    quantity = order->getQuantity();
    price = order->getIntPrice();
    isBuy = order->isBuy();
}

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::MarketOrderSubmitReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyPriceReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyQuantityReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderCancelReport& report) {
    // cancel report indicates the total deletion of an order
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return;
    timestamp = report.timestamp;
    messageType = MessageType::ORDER_DELETE;
    orderId = report.orderId;
    quantity = report.orderQuantity.value_or(0); // entire cancelled quantity
    price = Maths::castDoublePriceAsInt<uint32_t>(report.orderPrice.value_or(0.0));
    isBuy = report.orderSide == Market::Side::BUY;
}

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderPartialCancelReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return;
    timestamp = report.timestamp;
    messageType = MessageType::ORDER_CANCEL;
    orderId = report.orderId;
    quantity = report.cancelQuantity; // partially cancelled quantity
    price = Maths::castDoublePriceAsInt<uint32_t>(report.orderPrice.value_or(0.0));
    isBuy = report.orderSide == Market::Side::BUY;
}

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderCancelAndReplaceReport& /* report */) {} // TODO

std::string LobsterDataParser::OrderBookMessage::getAsCsv() const {
    return ""; // TODO
}

std::string LobsterDataParser::OrderBookSnapshot::getAsCsv() const {
    return ""; // TODO
}
}

#endif
