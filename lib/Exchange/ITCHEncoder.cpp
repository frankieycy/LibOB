#ifndef ITCH_ENCODER_CPP
#define ITCH_ENCODER_CPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Exchange {
using namespace Utils;

const std::string ITCHEncoder::ITCHSystemMessage::ourDescription                = "[S] Session start, end, or market open/close";
const std::string ITCHEncoder::ITCHOrderAddMessage::ourDescription              = "[A] New order entered into the book";
const std::string ITCHEncoder::ITCHOrderAddWithMPIDMessage::ourDescription      = "[F] Like A, but includes Market Participant ID";
const std::string ITCHEncoder::ITCHOrderExecuteMessage::ourDescription          = "[E] Order partially or fully filled; references the order ID and executed shares";
const std::string ITCHEncoder::ITCHOrderExecuteWithPriceMessage::ourDescription = "[C] Same as E, but includes execution price (for hidden orders)";
const std::string ITCHEncoder::ITCHOrderDeleteMessage::ourDescription           = "[D] Removes an order completely; ";
const std::string ITCHEncoder::ITCHOrderCancelMessage::ourDescription           = "[X] Cancels part of an order; includes order ref number and canceled shares";
const std::string ITCHEncoder::ITCHOrderReplaceMessage::ourDescription          = "[U] Cancel + add with a new order ref";
const std::string ITCHEncoder::ITCHTradeMessage::ourDescription                 = "[P] Regular trade execution";
const std::string ITCHEncoder::ITCHCrossTradeMessage::ourDescription            = "[Q] Used for open/close crosses";
const std::string ITCHEncoder::ITCHBrokenTradeMessage::ourDescription           = "[B] Trade bust (e.g. error correction)";

std::string to_string(const ITCHEncoder::EventCode& eventCode) {
    switch (eventCode) {
        case ITCHEncoder::EventCode::MARKET_OPEN:  return "O";
        case ITCHEncoder::EventCode::MARKET_CLOSE: return "C";
        default:                                   return "!";
    }
}

std::string to_string(const ITCHEncoder::MessageType& messageType) {
    switch (messageType) {
        case ITCHEncoder::MessageType::SYSTEM:                  return "S";
        case ITCHEncoder::MessageType::ORDER_ADD:               return "A";
        case ITCHEncoder::MessageType::ORDER_ADD_WITH_MPID:     return "F";
        case ITCHEncoder::MessageType::ORDER_EXECUTE:           return "E";
        case ITCHEncoder::MessageType::ORDER_EXECUTE_WITH_PRICE:return "C";
        case ITCHEncoder::MessageType::ORDER_DELETE:            return "D";
        case ITCHEncoder::MessageType::ORDER_CANCEL:            return "X";
        case ITCHEncoder::MessageType::ORDER_REPLACE:           return "U";
        case ITCHEncoder::MessageType::TRADE:                   return "P";
        case ITCHEncoder::MessageType::CROSS_TRADE:             return "Q";
        case ITCHEncoder::MessageType::BROKEN_TRADE:            return "B";
        default:                                                return "!";
    }
}

std::ostream& operator<<(std::ostream& out, const ITCHEncoder::EventCode& eventCode) { return out << to_string(eventCode); }

std::ostream& operator<<(std::ostream& out, const ITCHEncoder::MessageType& messageType) { return out << to_string(messageType); }

std::string ITCHEncoder::ITCHSystemMessage::toString() const {
    std::ostringstream oss;
    oss << "S|"
        << messageId << "|"
        << timestamp << "|"
        << agentId   << "|"
        << eventCode;
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderAddMessage::toString() const {
    std::ostringstream oss;
    oss << "A|"
        << messageId              << "|"
        << timestamp              << "|"
        << agentId                << "|"
        << std::string(symbol, 8) << "|"
        << orderId                << "|"
        << (isBuy ? 'B' : 'S')    << "|"
        << quantity               << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(price);
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderAddWithMPIDMessage::toString() const {
    std::ostringstream oss;
    oss << "F|"
        << messageId              << "|"
        << timestamp              << "|"
        << agentId                << "|"
        << std::string(symbol, 8) << "|"
        << orderId                << "|"
        << (isBuy ? 'B' : 'S')    << "|"
        << quantity               << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(price) << "|"
        << std::string(mpid, 4);
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderExecuteMessage::toString() const {
    std::ostringstream oss;
    oss << "E|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << orderId        << "|"
        << matchOrderId   << "|"
        << fillQuantity;
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderExecuteWithPriceMessage::toString() const {
    std::ostringstream oss;
    oss << "C|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << orderId        << "|"
        << matchOrderId   << "|"
        << fillQuantity   << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(fillPrice);
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderDeleteMessage::toString() const {
    std::ostringstream oss;
    oss << "D|"
        << messageId << "|"
        << timestamp << "|"
        << agentId   << "|"
        << orderId;
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderCancelMessage::toString() const {
    std::ostringstream oss;
    oss << "X|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << orderId        << "|"
        << cancelQuantity;
    return oss.str();
}

std::string ITCHEncoder::ITCHOrderReplaceMessage::toString() const {
    std::ostringstream oss;
    oss << "U|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << oldOrderId     << "|"
        << newOrderId     << "|"
        << quantity       << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(price);
    return oss.str();
}

std::string ITCHEncoder::ITCHTradeMessage::toString() const {
    std::ostringstream oss;
    oss << "P|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << orderId        << "|"
        << matchOrderId   << "|"
        << fillQuantity   << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(fillPrice);
    return oss.str();
}

std::string ITCHEncoder::ITCHCrossTradeMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHBrokenTradeMessage::toString() const {
    return ""; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderExecutionReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    if (report.orderType == Market::OrderType::LIMIT) {
        return std::make_shared<ITCHOrderExecuteWithPriceMessage>(
            report.reportId,
            report.timestamp,
            report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
            report.orderId,
            report.matchOrderId,
            report.filledQuantity,
            Maths::castDoublePriceAsInt<uint32_t>(report.filledPrice)
        );
    } else if (report.orderType == Market::OrderType::MARKET) {
        return std::make_shared<ITCHTradeMessage>(
            report.reportId,
            report.timestamp,
            report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
            report.orderId,
            report.matchOrderId,
            report.filledQuantity,
            Maths::castDoublePriceAsInt<uint32_t>(report.filledPrice)
        );
    }
    return nullptr;
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::LimitOrderSubmitReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    const auto& order = report.order;
    if (!order)
        Error::LIB_THROW("ITCHEncoder::encodeReport: LimitOrderSubmitReport order is null");
    const auto& metaInfo = order->getMetaInfo();
    return std::make_shared<ITCHOrderAddMessage>(
        report.reportId,
        report.timestamp,
        report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
        metaInfo ? metaInfo->getSymbolCharRaw() : ITCHEncoder::DEFAULT_SYMBOL,
        order->getId(),
        order->isBuy(),
        order->getQuantity(),
        order->getIntPrice()
    );
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::MarketOrderSubmitReport& /* report */) {
    // no encoding for market submit reports - relegated to execution reports
    return nullptr;
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyPriceReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyQuantityReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderCancelReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    return std::make_shared<ITCHOrderDeleteMessage>(
        report.reportId,
        report.timestamp,
        report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
        report.orderId
    );
}
}

#endif
