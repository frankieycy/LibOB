#ifndef ORDER_HPP
#define ORDER_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"

namespace Exchange {
class IMatchingEngine;
}

namespace Market {
using namespace Utils;
class OrderEventBase;
class OrderMetaInfo;

class OrderBase {
public:
    OrderBase();
    OrderBase(const OrderBase& order);
    OrderBase(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<const OrderMetaInfo>& metaInfo = nullptr);
    virtual ~OrderBase() = default;
    uint64_t getId() const { return myId; }
    uint64_t getTimestamp() const { return myTimestamp; }
    Side getSide() const { return mySide; }
    uint32_t getQuantity() const { return myQuantity; }
    OrderType getOrderType() const { return myOrderType; }
    OrderState getOrderState() const { return myOrderState; }
    std::shared_ptr<const OrderMetaInfo> getMetaInfo() const { return myMetaInfo; }
    bool isBuy() const { return mySide == Side::BUY; }
    bool isLimitOrder() const { return myOrderType == OrderType::LIMIT; }
    bool isAlive() const { return (myOrderState == OrderState::ACTIVE || myOrderState == OrderState::PARTIAL_FILLED) && myQuantity > 0; }
    void setId(const uint64_t id) { myId = id; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setSide(const Side side) { mySide = side; }
    void setQuantity(const uint32_t quantity) { myQuantity = quantity; }
    void setOrderType(const OrderType orderType) { myOrderType = orderType; }
    void setOrderState(const OrderState orderState) { myOrderState = orderState; }
    void setMetaInfo(const std::shared_ptr<const OrderMetaInfo>& metaInfo) { myMetaInfo = metaInfo; }
    virtual std::shared_ptr<OrderBase> clone() const { return std::make_shared<OrderBase>(*this); }
    virtual double getPrice() const { return Utils::Consts::NAN_DOUBLE; }
    virtual void executeOrderEvent(const OrderEventBase& /* event */) {}
    virtual void submit(Exchange::IMatchingEngine& /* matchingEngine */) const {}
    virtual bool checkState() const;
    virtual void init();
    virtual void cancel();
    virtual std::string getAsJson() const;
private:
    uint64_t myId;
    uint64_t myTimestamp;
    Side mySide;
    uint32_t myQuantity;
    OrderType myOrderType;
    OrderState myOrderState;
    std::shared_ptr<const OrderMetaInfo> myMetaInfo;
};

class LimitOrder : public OrderBase {
public:
    LimitOrder();
    LimitOrder(const LimitOrder& order);
    LimitOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const double price, const std::shared_ptr<OrderMetaInfo>& metaInfo = nullptr);
    virtual ~LimitOrder() = default;
    double getPrice() const override { return myPrice; }
    uint32_t getIntPrice() const { return myIntPrice; }
    void setPrice(const double price) {
        myPrice = price;
        myIntPrice = Maths::castDoublePriceAsInt<uint32_t>(price);
    }
    std::shared_ptr<LimitOrder> copy() const { return std::make_shared<LimitOrder>(*this); }
    virtual std::shared_ptr<OrderBase> clone() const override { return std::make_shared<LimitOrder>(*this); }
    virtual void executeOrderEvent(const OrderEventBase& event) override;
    virtual void submit(Exchange::IMatchingEngine& matchingEngine) const override;
    virtual bool checkState() const override;
    virtual void init() override;
    virtual void cancel() override;
    virtual std::string getAsJson() const override;
private:
    double myPrice;
    uint32_t myIntPrice; // for ITCH encoding
};

class MarketOrder : public OrderBase {
public:
    MarketOrder();
    MarketOrder(const MarketOrder& order);
    MarketOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo = nullptr);
    virtual ~MarketOrder() = default;
    std::shared_ptr<MarketOrder> copy() const { return std::make_shared<MarketOrder>(*this); }
    virtual std::shared_ptr<OrderBase> clone() const override { return std::make_shared<MarketOrder>(*this); }
    virtual void executeOrderEvent(const OrderEventBase& event) override;
    virtual void submit(Exchange::IMatchingEngine& matchingEngine) const override;
    virtual void init() override;
};

class HiddenOrder : public OrderBase {
public:
private:
};

std::ostream& operator<<(std::ostream& out, const OrderBase& order);
}

#endif
