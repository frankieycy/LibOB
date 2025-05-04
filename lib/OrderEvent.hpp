#ifndef ORDER_EVENT_HPP
#define ORDER_EVENT_HPP
#include "Utils.hpp"
#include "OrderUtils.hpp"

namespace Market {
class MarketOrder;
class LimitOrder;

class OrderEventBase {
public:
    OrderEventBase();
    OrderEventBase(const OrderEventBase& event);
    OrderEventBase(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp);
    virtual std::shared_ptr<OrderEventBase> clone() const { return std::make_shared<OrderEventBase>(*this); }
    const uint64_t getEventId() const { return myEventId; }
    const uint64_t getOrderId() const { return myOrderId; }
    const uint64_t getTimestamp() const { return myTimestamp; }
    const OrderEventType getEventType() const { return myEventType; }
    void setEventId(const uint64_t eventId) { myEventId = eventId; }
    void setOrderId(const uint64_t orderId) { myOrderId = orderId; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setEventType(const OrderEventType eventType) { myEventType = eventType; }
    virtual void applyTo(MarketOrder& order) const;
    virtual void applyTo(LimitOrder& order) const;
    virtual void init() {};
    virtual const std::string getAsJason() const;
    friend std::ostream& operator<<(std::ostream& out, const OrderEventBase& event);
private:
    uint64_t myEventId;
    uint64_t myOrderId;
    uint64_t myTimestamp;
    OrderEventType myEventType;
};

class OrderFillEvent : public OrderEventBase {
public:
    OrderFillEvent();
    OrderFillEvent(const OrderFillEvent& event);
    OrderFillEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const int fillQuantity, const double fillPrice);
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderFillEvent>(*this); }
    const int getFillQuantity() const { return myFillQuantity; }
    const double getFillPrice() const { return myFillPrice; }
    void setFillQuantity(const int fillQuantity) { myFillQuantity = fillQuantity; }
    void setFillPrice(const int fillPrice) { myFillPrice = fillPrice; }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJason() const override;
private:
    int myFillQuantity;
    double myFillPrice;
};

class OrderModifyPriceEvent : public OrderEventBase {
public:
    OrderModifyPriceEvent();
    OrderModifyPriceEvent(const OrderModifyPriceEvent& event);
    OrderModifyPriceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedPrice);
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyPriceEvent>(*this); }
    const double getModifiedPrice() const { return myModifiedPrice; }
    void setModifiedPrice(const int modifiedPrice) { myModifiedPrice = modifiedPrice; }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJason() const override;
private:
    double myModifiedPrice;
};

class OrderModifyQuantityEvent : public OrderEventBase {
public:
    OrderModifyQuantityEvent();
    OrderModifyQuantityEvent(const OrderModifyQuantityEvent& event);
    OrderModifyQuantityEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity);
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyQuantityEvent>(*this); }
    const int getModifiedQuantity() const { return myModifiedQuantity; }
    void setModifiedQuantity(const int modifiedQuantity) { myModifiedQuantity = modifiedQuantity; }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJason() const override;
private:
    int myModifiedQuantity;
};

class OrderCancelEvent : public OrderEventBase {
public:
    OrderCancelEvent();
    OrderCancelEvent(const OrderCancelEvent& event);
    OrderCancelEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity);
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderCancelEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
};
}

#endif
