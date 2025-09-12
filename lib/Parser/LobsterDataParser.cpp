#ifndef LOBSTER_DATA_PARSER_CPP
#define LOBSTER_DATA_PARSER_CPP
#include "LobsterDataParser.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

namespace Parser {
LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderExecutionReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::LimitOrderSubmitReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::MarketOrderSubmitReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyPriceReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderModifyQuantityReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderCancelReport& /* report */) {} // TODO

LobsterDataParser::OrderBookMessage::OrderBookMessage(const Exchange::OrderCancelAndReplaceReport& /* report */) {} // TODO

std::string LobsterDataParser::OrderBookMessage::getAsCsv() const {
    return ""; // TODO
}

std::string LobsterDataParser::OrderBookSnapshot::getAsCsv() const {
    return ""; // TODO
}
}

#endif
