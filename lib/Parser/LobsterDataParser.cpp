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

// limit order submit only indicates the reception of the order, and placement report must appear afterwards
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::LimitOrderSubmitReport& /* report */) : OrderBookMessage() {}

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::LimitOrderPlacementReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return;
    timestamp = report.timestamp;
    messageType = MessageType::ORDER_ADD;
    orderId = report.orderId;
    quantity = report.orderQuantity;
    price = Maths::castDoublePriceAsInt<uint32_t>(report.orderPrice);
    isBuy = report.orderSide == Market::Side::BUY;
}

// market order submit implies immediate execution, and execution report must appear afterwards
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::MarketOrderSubmitReport& /* report */) : OrderBookMessage() {}

// represented as delete + add in Lobster format
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyPriceReport& /* report */) : OrderBookMessage(true) {}

// represented as delete + add in Lobster format
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyQuantityReport& /* report */) : OrderBookMessage(true) {}

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

// represented as delete + add in Lobster format
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderCancelAndReplaceReport& /* report */) : OrderBookMessage(true) {}

std::string LobsterDataParser::OrderBookMessage::getAsCsv(bool aligned) const {
    std::ostringstream oss;
    if (aligned) {
        oss << std::setw(8) << timestamp << ","
            << std::setw(8) << +static_cast<uint8_t>(messageType) << ","
            << std::setw(8) << orderId << ","
            << std::setw(8) << quantity << ","
            << std::setw(8) << price << ","
            << std::setw(8) << (isBuy ? 1 : -1);
    } else {
        oss << timestamp << ","
            << +static_cast<uint8_t>(messageType) << ","
            << orderId << ","
            << quantity << ","
            << price << ","
            << (isBuy ? 1 : -1);
    }
    return oss.str();
}

std::string LobsterDataParser::OrderBookSnapshot::getAsCsv(size_t levels, bool aligned) const {
    std::ostringstream oss;
    if (aligned) {
        for (size_t i = 0; i < levels; ++i) {
            if (i < askPrice.size())
                oss << std::setw(8) << askPrice[i] << "," << std::setw(8) << askSize[i] << ",";
            else
                oss << std::setw(8) << "" << "," << std::setw(8) << "" << ",";
            if (i < bidPrice.size())
                oss << std::setw(8) << bidPrice[i] << "," << std::setw(8) << bidSize[i];
            else
                oss << std::setw(8) << "" << "," << std::setw(8) << "";
            if (i != levels - 1)
                oss << ",";
        }
    } else {
        for (size_t i = 0; i < levels; ++i) {
            if (i < askPrice.size())
                oss << askPrice[i] << "," << askSize[i] << ",";
            else
                oss << ",,";
            if (i < bidPrice.size())
                oss << bidPrice[i] << "," << bidSize[i];
            else
                oss << ",";
            if (i != levels - 1)
                oss << ",";
        }
    }
    return oss.str();
}
}

#endif
