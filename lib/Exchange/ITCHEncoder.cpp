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

std::string ITCHEncoder::ITCHSystemMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderAddMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderAddWithMPIDMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderExecuteMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderExecuteWithPriceMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderDeleteMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderCancelMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHOrderReplaceMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHTradeMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHCrossTradeMessage::toString() const {
    return ""; // TODO
}

std::string ITCHEncoder::ITCHBrokenTradeMessage::toString() const {
    return ""; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderExecutionReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::LimitOrderSubmitReport& report) {
    const auto& order = report.order;
    if (!order)
        Error::LIB_THROW("ITCHEncoder::encodeReport: LimitOrderSubmitReport order is null");
    const auto& metaInfo = order->getMetaInfo();
    return std::make_shared<ITCHOrderAddMessage>(
        report.reportId,
        report.timestamp,
        String::hashStringTo<uint64_t>(metaInfo ? metaInfo->getAgentId() : ""),
        "", // TODO: 8-char padded symbol
        order->getId(),
        order->isBuy(),
        order->getQuantity(),
        static_cast<uint32_t>(order->getPrice() * 10000)
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

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderCancelReport& /* report */) {
    return nullptr; // TODO
}
}

#endif
