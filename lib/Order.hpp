#ifndef ORDER_HPP
#define ORDER_HPP
#include "Utils.hpp"
#include "OrderUtils.hpp"

namespace Exchange {
class IMatchingEngine;
}

namespace Market {
class OrderEventBase;
class OrderMetaInfo;

class OrderBase {
public:
    OrderBase();
    OrderBase(const OrderBase& order);
    OrderBase(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo = nullptr);
    const uint64_t getId() const { return myId; }
    const uint64_t getTimestamp() const { return myTimestamp; }
    const Side getSide() const { return mySide; }
    const uint32_t getQuantity() const { return myQuantity; }
    const OrderType getOrderType() const { return myOrderType; }
    const OrderState getOrderState() const { return myOrderState; }
    const std::shared_ptr<OrderMetaInfo> getMetaInfo() const { return myMetaInfo; }
    const bool isBuy() const { return mySide == Side::BUY; }
    const bool isActive() const { return myOrderState == OrderState::ACTIVE; }
    void setId(const uint64_t id) { myId = id; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setSide(const Side side) { mySide = side; }
    void setQuantity(const uint32_t quantity) { myQuantity = quantity; }
    void setOrderType(const OrderType orderType) { myOrderType = orderType; }
    void setOrderState(const OrderState orderState) { myOrderState = orderState; }
    void setMetaInfo(const std::shared_ptr<OrderMetaInfo>& metaInfo) { myMetaInfo = metaInfo; }
    virtual std::shared_ptr<OrderBase> clone() const { return std::make_shared<OrderBase>(*this); }
    virtual void executeOrderEvent(const OrderEventBase& event) {}
    virtual void submit(Exchange::IMatchingEngine& matchingEngine) const {}
    virtual void init();
    virtual void cancel();
    virtual const std::string getAsJason() const;
    friend std::ostream& operator<<(std::ostream& out, const OrderBase& order);
private:
    uint64_t myId;
    uint64_t myTimestamp;
    Side mySide;
    uint32_t myQuantity;
    OrderType myOrderType;
    OrderState myOrderState;
    std::shared_ptr<OrderMetaInfo> myMetaInfo;
};

class LimitOrder : public OrderBase {
public:
    LimitOrder();
    LimitOrder(const LimitOrder& order);
    LimitOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const double price, const std::shared_ptr<OrderMetaInfo>& metaInfo = nullptr);
    const double getPrice() const { return myPrice; }
    void setPrice(const double price) { myPrice = price; }
    virtual std::shared_ptr<OrderBase> clone() const override { return std::make_shared<LimitOrder>(*this); }
    virtual void executeOrderEvent(const OrderEventBase& event) override;
    virtual void submit(Exchange::IMatchingEngine& matchingEngine) const override;
    virtual void init() override;
    virtual void cancel() override;
    virtual const std::string getAsJason() const override;
private:
    double myPrice;
};

class MarketOrder : public OrderBase {
public:
    MarketOrder();
    MarketOrder(const MarketOrder& order);
    MarketOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo = nullptr);
    virtual std::shared_ptr<OrderBase> clone() const override { return std::make_shared<MarketOrder>(*this); }
    virtual void executeOrderEvent(const OrderEventBase& event) override;
    virtual void submit(Exchange::IMatchingEngine& matchingEngine) const override;
    virtual void init() override;
};

class HiddenOrder : public OrderBase {
public:
private:
};
}

#endif
