#ifndef META_INFO_HPP
#define META_INFO_HPP
#include "Utils.hpp"

namespace Market {
class TradeMetaInfo {
public:
    TradeMetaInfo() = default;
    TradeMetaInfo(const TradeMetaInfo& info);
    TradeMetaInfo(const std::string symbol, const std::string exchangeId);
    const std::string getSymbol() const { return mySymbol; }
    const std::string getExchangeId() const { return myExchangeId; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    virtual std::shared_ptr<TradeMetaInfo> clone() const { return std::make_shared<TradeMetaInfo>(*this); }
    virtual const std::string getAsJson() const;
    friend std::ostream& operator<<(std::ostream& out, const TradeMetaInfo& order);
private:
    std::string mySymbol;
    std::string myExchangeId;
};

class OrderMetaInfo : public TradeMetaInfo {
public:
    OrderMetaInfo() = default;
    OrderMetaInfo(const OrderMetaInfo& info);
    OrderMetaInfo(const std::string symbol, const std::string exchangeId, const std::string agentId);
    const std::string getAgentId() const { return myAgentId; }
    void setAgentId(const std::string& agentId) { myAgentId = agentId; }
    virtual std::shared_ptr<TradeMetaInfo> clone() const override { return std::make_shared<OrderMetaInfo>(*this); }
    virtual const std::string getAsJson() const override;
    friend std::ostream& operator<<(std::ostream& out, const OrderMetaInfo& order);
private:
    std::string myAgentId;
};
}

#endif
