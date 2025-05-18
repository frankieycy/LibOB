#ifndef TRADE_HPP
#define TRADE_HPP
#include "Utils.hpp"
#include "MetaInfo.hpp"

namespace Market {
class OrderBase;

class TradeBase {
public:
    TradeBase();
    TradeBase(const TradeBase& trade);
    TradeBase(
        const uint64_t id, const uint64_t timestamp, const uint64_t buyId, const uint64_t sellId, const uint32_t quantity, const double price,
        const bool isBuyLimitOrder, const bool isSellLimitOrder, const bool isBuyInitiated, const std::shared_ptr<const TradeMetaInfo>& metaInfo = nullptr);
    TradeBase(const uint64_t id, const uint64_t timestamp, const uint32_t quantity, const double price, const bool isBuyInitiated, const OrderBase& buyOrder, const OrderBase& sellOrder);
    virtual ~TradeBase() = default;
    uint64_t getId() const { return myId; }
    uint64_t getTimestamp() const { return myTimestamp; }
    uint64_t getBuyId() const { return myBuyId; }
    uint64_t getSellId() const { return mySellId; }
    uint32_t getQuantity() const { return myQuantity; }
    double getPrice() const { return myPrice; }
    bool getIsBuyLimitOrder() const { return myIsBuyLimitOrder; }
    bool getIsSellLimitOrder() const { return myIsSellLimitOrder; }
    bool getIsBuyInitiated() const { return myIsBuyInitiated; }
    std::string getSymbol() const { return myMetaInfo ? myMetaInfo->getSymbol() : ""; }
    std::string getExchangeId() const { return myMetaInfo ? myMetaInfo->getExchangeId() : ""; }
    void setId(const uint64_t id) { myId = id; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setBuyId(const uint64_t buyId) { myBuyId = buyId; }
    void setSellId(const uint64_t sellId) { mySellId = sellId; }
    void setQuantity(const uint32_t quantity) { myQuantity = quantity; }
    void setPrice(const double price) { myPrice = price; }
    void setIsBuyLimitOrder(const bool isBuyLimitOrder) { myIsBuyLimitOrder = isBuyLimitOrder; }
    void setIsSellLimitOrder(const bool isSellLimitOrder) { myIsSellLimitOrder = isSellLimitOrder; }
    void setIsBuyInitiated(const bool isBuyInitiated) { myIsBuyInitiated = isBuyInitiated; }
    virtual std::shared_ptr<TradeBase> clone() const { return std::make_shared<TradeBase>(*this); }
    virtual void init();
    virtual std::string getAsJson() const;
private:
    uint64_t myId;
    uint64_t myTimestamp;
    uint64_t myBuyId;
    uint64_t mySellId;
    uint32_t myQuantity;
    double myPrice;
    bool myIsBuyLimitOrder;
    bool myIsSellLimitOrder;
    bool myIsBuyInitiated;
    std::shared_ptr<const TradeMetaInfo> myMetaInfo;
};

std::ostream& operator<<(std::ostream& out, const TradeBase& trade);
}

#endif
