#ifndef TRADE_HPP
#define TRADE_HPP
#include "Utils.hpp"
#include "MetaInfo.hpp"
#include "Order.hpp"

namespace Market {
class TradeBase {
public:
    TradeBase();
    TradeBase(
        const uint64_t id, const uint64_t timestamp, const uint64_t buyId, const uint64_t sellId, const int quantity, const double price,
        const bool isBuyLimitOrder, const bool isSellLimitOrder, const bool isBuyInitiated, const std::shared_ptr<TradeMetaInfo>& metaInfo);
    TradeBase(const uint64_t id, const uint64_t timestamp, const int quantity, const double price, const bool isBuyInitiated, const OrderBase& buyOrder, const OrderBase& sellOrder);
    TradeBase(const TradeBase& trade);
    const uint64_t getId() const { return myId; }
    const uint64_t getTimestamp() const { return myTimestamp; }
    const uint64_t getBuyId() const { return myBuyId; }
    const uint64_t getSellId() const { return mySellId; }
    const int getQuantity() const { return myQuantity; }
    const double getPrice() const { return myPrice; }
    const bool getIsBuyLimitOrder() const { return myIsBuyLimitOrder; }
    const bool getIsSellLimitOrder() const { return myIsSellLimitOrder; }
    const bool getIsBuyInitiated() const { return myIsBuyInitiated; }
    const std::string getSymbol() const { return myMetaInfo->getSymbol(); }
    const std::string getExchangeId() const { return myMetaInfo->getExchangeId(); }
    void setId(const uint64_t id) {myId = id; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setBuyId(const uint64_t buyId) { myBuyId = buyId; }
    void setSellId(const uint64_t sellId) { mySellId = sellId; }
    void setQuantity(const int quantity) { myQuantity = quantity; }
    void setPrice(const double price) { myPrice = price; }
    void setIsBuyLimitOrder(const bool isBuyLimitOrder) { myIsBuyLimitOrder = isBuyLimitOrder; }
    void setIsSellLimitOrder(const bool isSellLimitOrder) { myIsSellLimitOrder = isSellLimitOrder; }
    void setIsBuyInitiated(const bool isBuyInitiated) { myIsBuyInitiated = isBuyInitiated; }
    virtual void init();
    virtual std::shared_ptr<TradeBase> clone() const { return std::make_shared<TradeBase>(*this); }
    virtual const std::string getAsJason() const;
    friend std::ostream& operator<<(std::ostream& out, const TradeBase& trade);
private:
    uint64_t myId;
    uint64_t myTimestamp;
    uint64_t myBuyId;
    uint64_t mySellId;
    int myQuantity;
    double myPrice;
    bool myIsBuyLimitOrder;
    bool myIsSellLimitOrder;
    bool myIsBuyInitiated;
    std::shared_ptr<TradeMetaInfo> myMetaInfo;
};
}

#endif
