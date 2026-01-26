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
    const char* getSymbolCharRaw() const { return mySymbolCharRaw; }
    const char* getExchangeIdCharRaw() const { return myExchangeIdCharRaw; }
    void setSymbol(const std::string& symbol) {
        mySymbol = symbol;
        Utils::String::stringToCharRaw(symbol, mySymbolCharRaw, '0');
    }
    void setExchangeId(const std::string& exchangeId) {
        myExchangeId = exchangeId;
        Utils::String::stringToCharRaw(exchangeId, myExchangeIdCharRaw, '0');
    }
    virtual std::shared_ptr<TradeMetaInfo> clone() const { return std::make_shared<TradeMetaInfo>(*this); }
    virtual void init();
    virtual std::string getAsJson() const;
private:
    std::string mySymbol;
    std::string myExchangeId;
    char mySymbolCharRaw[8]; // for ITCH encoding
    char myExchangeIdCharRaw[8];
};

class OrderMetaInfo : public TradeMetaInfo {
public:
    OrderMetaInfo() = default;
    OrderMetaInfo(const OrderMetaInfo& info);
    OrderMetaInfo(const std::string symbol, const std::string exchangeId, const std::string agentId, const std::string marketParticipantId);
    virtual ~OrderMetaInfo() = default;
    std::string getAgentId() const { return myAgentId; }
    std::string getMarketParticipantId() const { return myMarketParticipantId; }
    uint64_t getAgentIdHash() const { return myAgentIdHash; }
    const char* getMarketParticipantCharRaw() const { return myMarketParticipantIdCharRaw; }
    void setAgentId(const std::string& agentId) {
        myAgentId = agentId;
        myAgentIdHash = Utils::String::hashStringTo<uint64_t>(agentId);
    }
    void setMarketParticipantId(const std::string& marketParticipantId) {
        myMarketParticipantId = marketParticipantId;
        Utils::String::stringToCharRaw(marketParticipantId, myMarketParticipantIdCharRaw, '0');
    }
    virtual std::shared_ptr<TradeMetaInfo> clone() const override { return std::make_shared<OrderMetaInfo>(*this); }
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    std::string myAgentId;
    std::string myMarketParticipantId;
    uint64_t myAgentIdHash; // for ITCH encoding
    char myMarketParticipantIdCharRaw[4];
};

std::ostream& operator<<(std::ostream& out, const TradeMetaInfo& order);
}

#endif
