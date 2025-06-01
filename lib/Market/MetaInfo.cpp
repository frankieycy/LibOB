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
    myExchangeId(info.myExchangeId) {}

TradeMetaInfo::TradeMetaInfo(const std::string symbol, const std::string exchangeId) :
    mySymbol(symbol),
    myExchangeId(exchangeId) {}

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
    myAgentId(info.myAgentId) {}

OrderMetaInfo::OrderMetaInfo(const std::string symbol, const std::string exchangeId, const std::string agentId, const std::string marketParticipantId) :
    TradeMetaInfo(symbol, exchangeId),
    myAgentId(agentId),
    myMarketParticipantId(marketParticipantId) {}

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
