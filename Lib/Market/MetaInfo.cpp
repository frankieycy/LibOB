#ifndef META_INFO_CPP
#define META_INFO_CPP
#include "Utils/Utils.hpp"
#include "Market/MetaInfo.hpp"

namespace Market {
std::ostream& operator<<(std::ostream& out, const TradeMetaInfo& metaInfo) {
    return out << metaInfo.getAsJson();
}

TradeMetaInfo::TradeMetaInfo(const TradeMetaInfo& info) :
    mySymbol(info.mySymbol),
    myExchangeId(info.myExchangeId) {
    init();
}

TradeMetaInfo::TradeMetaInfo(const std::string symbol, const std::string exchangeId) :
    mySymbol(symbol),
    myExchangeId(exchangeId) {
    init();
}

void TradeMetaInfo::init() {
    Utils::String::stringToCharRaw(mySymbol, mySymbolCharRaw, '0');
    Utils::String::stringToCharRaw(myExchangeId, myExchangeIdCharRaw, '0');
}

std::string TradeMetaInfo::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"Symbol\":\""     << getSymbol()     << "\","
    "\"ExchangeId\":\"" << getExchangeId() << "\""
    "}";
    return oss.str();
}

OrderMetaInfo::OrderMetaInfo(const OrderMetaInfo& info) :
    TradeMetaInfo(info),
    myAgentId(info.myAgentId),
    myMarketParticipantId(info.myMarketParticipantId) {
    init();
}

OrderMetaInfo::OrderMetaInfo(const std::string symbol, const std::string exchangeId, const std::string agentId, const std::string marketParticipantId) :
    TradeMetaInfo(symbol, exchangeId),
    myAgentId(agentId),
    myMarketParticipantId(marketParticipantId) {
    init();
}

void OrderMetaInfo::init() {
    myAgentIdHash = Utils::String::hashStringTo<uint64_t>(myAgentId);
    Utils::String::stringToCharRaw(myMarketParticipantId, myMarketParticipantIdCharRaw, '0');
}

std::string OrderMetaInfo::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"Symbol\":\""     << getSymbol()     << "\","
    "\"ExchangeId\":\"" << getExchangeId() << "\","
    "\"getAgentId\":\"" << getAgentId()    << "\""
    "}";
    return oss.str();
}
}

#endif
