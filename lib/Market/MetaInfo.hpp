#ifndef META_INFO_HPP
#define META_INFO_HPP
#include "Utils/Utils.hpp"

namespace Market {
class TradeMetaInfo {
public:
    TradeMetaInfo() = default;
    TradeMetaInfo(const TradeMetaInfo& info);
    TradeMetaInfo(const std::string symbol, const std::string exchangeId);
    virtual ~TradeMetaInfo() = default;
    std::string getSymbol() const { return mySymbol; }
    std::string getExchangeId() const { return myExchangeId; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    virtual std::shared_ptr<TradeMetaInfo> clone() const { return std::make_shared<TradeMetaInfo>(*this); }
    virtual std::string getAsJson() const;
private:
    std::string mySymbol;
    std::string myExchangeId;
};

class OrderMetaInfo : public TradeMetaInfo {
public:
    OrderMetaInfo() = default;
    OrderMetaInfo(const OrderMetaInfo& info);
    OrderMetaInfo(const std::string symbol, const std::string exchangeId, const std::string agentId, const std::string marketParticipantId);
    virtual ~OrderMetaInfo() = default;
    std::string getAgentId() const { return myAgentId; }
    std::string getMarketParticipantId() const { return myMarketParticipantId; }
    void setAgentId(const std::string& agentId) { myAgentId = agentId; }
    void setMarketParticipantId(const std::string& marketParticipantId) { myMarketParticipantId = marketParticipantId; }
    virtual std::shared_ptr<TradeMetaInfo> clone() const override { return std::make_shared<OrderMetaInfo>(*this); }
    virtual std::string getAsJson() const override;
private:
    std::string myAgentId;
    std::string myMarketParticipantId;
};

std::ostream& operator<<(std::ostream& out, const TradeMetaInfo& order);
}

#endif
