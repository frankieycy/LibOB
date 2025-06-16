#ifndef ITCH_ENCODER_CPP
#define ITCH_ENCODER_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"
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

std::string to_string(const ITCHEncoder::CrossCode& crossCode) {
    switch (crossCode) {
        case ITCHEncoder::CrossCode::OPENING: return "O";
        case ITCHEncoder::CrossCode::CLOSING: return "C";
        case ITCHEncoder::CrossCode::HALT:    return "H";
        case ITCHEncoder::CrossCode::IPO:     return "I";
        default:                              return "!";
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

std::ostream& operator<<(std::ostream& out, const ITCHEncoder::CrossCode& crossCode) { return out << to_string(crossCode); }

std::ostream& operator<<(std::ostream& out, const ITCHEncoder::MessageType& messageType) { return out << to_string(messageType); }

std::ostream& operator<<(std::ostream& out, const ITCHEncoder::ITCHMessage& message) { return out << message.toString(); }

std::string ITCHEncoder::ITCHSystemMessage::toString() const {
    std::ostringstream oss;
    oss << "S|"
        << messageId << "|"
        << timestamp << "|"
        << agentId   << "|"
        << eventCode;
    return oss.str();
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderAddMessage::makeEvent() const {
    const auto metaInfo = std::make_shared<Market::OrderMetaInfo>(symbol, "", std::to_string(agentId), "");
    const auto order = std::make_shared<Market::LimitOrder>(orderId, timestamp, isBuy ? Market::Side::BUY : Market::Side::SELL, quantity, Maths::castIntPriceAsDouble(price));
    return std::make_shared<Market::OrderSubmitEvent>(messageId, orderId, timestamp, order);
}

std::string ITCHEncoder::ITCHOrderAddMessage::toString() const {
    std::ostringstream oss;
    oss << "A|"
        << messageId              << "|"
        << timestamp              << "|"
        << agentId                << "|"
        << std::string(symbol)    << "|"
        << orderId                << "|"
        << (isBuy ? 'B' : 'S')    << "|"
        << quantity               << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(price);
    return oss.str();
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderAddWithMPIDMessage::makeEvent() const {
    const auto metaInfo = std::make_shared<Market::OrderMetaInfo>(symbol, "", std::to_string(agentId), mpid);
    const auto order = std::make_shared<Market::LimitOrder>(orderId, timestamp, isBuy ? Market::Side::BUY : Market::Side::SELL, quantity, Maths::castIntPriceAsDouble(price));
    return std::make_shared<Market::OrderSubmitEvent>(messageId, orderId, timestamp, order);
}

std::string ITCHEncoder::ITCHOrderAddWithMPIDMessage::toString() const {
    std::ostringstream oss;
    oss << "F|"
        << messageId              << "|"
        << timestamp              << "|"
        << agentId                << "|"
        << std::string(symbol)    << "|"
        << orderId                << "|"
        << (isBuy ? 'B' : 'S')    << "|"
        << quantity               << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(price) << "|"
        << std::string(mpid, 4);
    return oss.str();
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderExecuteMessage::makeEvent() const {
    // The execute message informs that a maker order on the book was filled by a taker (market) order,
    // and the message must come with a complementary trade message from which the taker order submit
    // can be inferred. Thus, we do not create an event here.
    return nullptr;
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

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderExecuteWithPriceMessage::makeEvent() const {
    // Same as ITCHOrderExecuteMessage, we do not create an event here.
    return nullptr;
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

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderDeleteMessage::makeEvent() const {
    return std::make_shared<Market::OrderCancelEvent>(messageId, orderId, timestamp);
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

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderCancelMessage::makeEvent() const {
    return std::make_shared<Market::OrderPartialCancelEvent>(messageId, orderId, timestamp, cancelQuantity);
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

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHOrderReplaceMessage::makeEvent() const {
    return std::make_shared<Market::OrderCancelAndReplaceEvent>(messageId, oldOrderId, timestamp, newOrderId, quantity, Maths::castIntPriceAsDouble(price));
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

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHTradeMessage::makeEvent() const {
    // The trade message does not contain symbol and side, hence insufficient to construct a market submit event.
    // The client (e.g. order event manger) may identify the trade message by calling isOrderOperation() and getMatchOrderId()
    // to locate the match order internally stored, then flip the side and copy the symbol to construct the market submit event.
    return nullptr;
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
    std::ostringstream oss;
    oss << "Q|"
        << messageId           << "|"
        << timestamp           << "|"
        << agentId             << "|"
        << std::string(symbol) << "|"
        << crossQuantity       << "|"
        << std::fixed << std::setprecision(2) << Maths::castIntPriceAsDouble(crossPrice) << "|"
        << crossCode;
    return oss.str();
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::ITCHBrokenTradeMessage::makeEvent() const {
    return nullptr; // TODO
}

std::string ITCHEncoder::ITCHBrokenTradeMessage::toString() const {
    std::ostringstream oss;
    oss << "B|"
        << messageId      << "|"
        << timestamp      << "|"
        << agentId        << "|"
        << matchOrderId;
    return oss.str();
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

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyPriceReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    return std::make_shared<ITCHOrderReplaceMessage>(
        report.reportId,
        report.timestamp,
        report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
        report.orderId,
        report.orderId,
        report.orderQuantity,
        Maths::castDoublePriceAsInt<uint32_t>(report.modifiedPrice)
    );
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyQuantityReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    return std::make_shared<ITCHOrderReplaceMessage>(
        report.reportId,
        report.timestamp,
        report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
        report.orderId,
        report.orderId,
        report.modifiedQuantity,
        Maths::castDoublePriceAsInt<uint32_t>(report.orderPrice)
    );
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

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderCancelAndReplaceReport& report) {
    if (report.status != Exchange::OrderProcessingStatus::SUCCESS)
        return nullptr;
    return std::make_shared<ITCHOrderReplaceMessage>(
        report.reportId,
        report.timestamp,
        report.agentIdHash.value_or(ITCHEncoder::DEFAULT_AGENT_ID),
        report.orderId,
        report.newOrderId,
        report.newQuantity,
        Maths::castDoublePriceAsInt<uint32_t>(report.newPrice)
    );
}
}

#endif
