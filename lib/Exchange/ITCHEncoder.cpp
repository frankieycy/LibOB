#ifndef ITCH_ENCODER_CPP
#define ITCH_ENCODER_CPP
#include "Utils/Utils.hpp"
#include "Market/MetaInfo.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Exchange {
using namespace Utils;

const std::unordered_map<ITCHEncoder::MessageType, std::pair<ITCHEncoder::MessageEncoding, std::string>> ITCHEncoder::MESSAGE_TYPE_ENCODINGS = {
    { ITCHEncoder::MessageType::SYSTEM,                   { ITCHEncoder::MessageEncoding::S, "[S] System Message - session start, end, or market open/close" } },
    { ITCHEncoder::MessageType::ORDER_ADD,                { ITCHEncoder::MessageEncoding::A, "[A] New order entered into the book" } },
    { ITCHEncoder::MessageType::ORDER_ADD_WITH_MPID,      { ITCHEncoder::MessageEncoding::F, "[F] Like A, but includes Market Participant ID" } },
    { ITCHEncoder::MessageType::ORDER_EXECUTE,            { ITCHEncoder::MessageEncoding::E, "[E] Order partially or fully filled; references the order ID and executed shares" } },
    { ITCHEncoder::MessageType::ORDER_EXECUTE_WITH_PRICE, { ITCHEncoder::MessageEncoding::C, "[C] Same as E, but includes execution price (for hidden orders) "} },
    { ITCHEncoder::MessageType::ORDER_CANCEL,             { ITCHEncoder::MessageEncoding::X, "[X] Cancels part of an order; includes order ref number and canceled shares" } },
    { ITCHEncoder::MessageType::ORDER_DELETE,             { ITCHEncoder::MessageEncoding::D, "[D] Removes an order completely" } },
    { ITCHEncoder::MessageType::ORDER_REPLACE,            { ITCHEncoder::MessageEncoding::U, "[U] Cancel + add with a new order ref" } },
    { ITCHEncoder::MessageType::TRADE,                    { ITCHEncoder::MessageEncoding::P, "[P] Regular trade execution" } },
    { ITCHEncoder::MessageType::CROSS_TRADE,              { ITCHEncoder::MessageEncoding::Q, "[Q] Used for open/close crosses" } },
    { ITCHEncoder::MessageType::BROKEN_TRADE,             { ITCHEncoder::MessageEncoding::B, "[B] Trade bust (e.g. error correction)" } }
};

std::string ITCHEncoder::encode(const Exchange::OrderExecutionReport& /* report */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Exchange::LimitOrderSubmitReport& report) {
    // ORDER_ADD: "A|timestamp|orderId|B/S|quantity|price|symbol"
    std::ostringstream oss;
    const auto& order = report.order;
    const auto& metaInfo = order->getMetaInfo();
    if (!order)
        Error::LIB_THROW("[ITCHEncoder::encode] Order is null in LimitOrderSubmitReport.");
    if (!order->isLimitOrder())
        Error::LIB_THROW("[ITCHEncoder::encode] Order is not a limit order in LimitOrderSubmitReport.");
    oss << "A|"
        << report.timestamp             << "|"
        << report.orderId               << "|"
        << (order->isBuy() ? "B" : "S") << "|"
        << order->getQuantity()         << "|"
        << order->getPrice()            << "|"
        << (metaInfo ? metaInfo->getSymbol() : "");
    return oss.str();
}

std::string ITCHEncoder::encode(const Exchange::MarketOrderSubmitReport& /* report */) {
    // no encoding for market order submit
    return "";
}

std::string ITCHEncoder::encode(const Exchange::OrderModifyPriceReport& /* report */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Exchange::OrderModifyQuantityReport& /* report */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Exchange::OrderCancelReport& report) {
    // ORDER_DELETE: "D|timestamp|orderId"
    std::ostringstream oss;
    oss << "D|"
        << report.timestamp << "|"
        << report.orderId;
    return oss.str();
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::OrderExecutionReport& /* report */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::LimitOrderSubmitReport& /* report */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::MarketOrderSubmitReport& /* report */) {
    // no encoding for market order submit
    return {};
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::OrderModifyPriceReport& /* report */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::OrderModifyQuantityReport& /* report */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Exchange::OrderCancelReport& /* report */) {
    return {}; // TODO
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::decodeStringMessage(const std::string& /* message */) {
    return nullptr; // TODO
}

std::shared_ptr<Market::OrderEventBase> ITCHEncoder::decodeBinaryMessage(const std::vector<uint8_t>& /* message */) {
    return nullptr; // TODO
}
}

#endif
