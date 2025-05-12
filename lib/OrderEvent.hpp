#ifndef ORDER_EVENT_HPP
#define ORDER_EVENT_HPP
#include "Utils.hpp"
#include "OrderUtils.hpp"

namespace Market {
class OrderBase;
class MarketOrder;
class LimitOrder;

class OrderEventBase {
public:
    OrderEventBase();
    OrderEventBase(const OrderEventBase& event);
    OrderEventBase(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<OrderBase>& order = nullptr);
    const uint64_t getEventId() const { return myEventId; }
    const uint64_t getOrderId() const { return myOrderId; }
    const uint64_t getTimestamp() const { return myTimestamp; }
    const OrderEventType getEventType() const { return myEventType; }
    const std::shared_ptr<OrderBase>& getOrder() const { return myOrder; }
    const bool isSubmit() const { return myEventType == OrderEventType::SUBMIT; }
    void setEventId(const uint64_t eventId) { myEventId = eventId; }
    void setOrderId(const uint64_t orderId) { myOrderId = orderId; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setEventType(const OrderEventType eventType) { myEventType = eventType; }
    void setOrder(const std::shared_ptr<OrderBase>& order) { myOrder = order; }
    virtual std::shared_ptr<OrderEventBase> clone() const { return std::make_shared<OrderEventBase>(*this); }
    virtual void applyTo(MarketOrder& order) const;
    virtual void applyTo(LimitOrder& order) const;
    virtual void init() {};
    virtual const std::string getAsJson() const;
    friend std::ostream& operator<<(std::ostream& out, const OrderEventBase& event);
private:
    uint64_t myEventId;
    uint64_t myOrderId;
    uint64_t myTimestamp;
    OrderEventType myEventType;
    std::shared_ptr<OrderBase> myOrder;
};

class OrderSubmitEvent : public OrderEventBase {
public:
    OrderSubmitEvent();
    OrderSubmitEvent(const OrderSubmitEvent& event);
    OrderSubmitEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<OrderBase>& order);
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderSubmitEvent>(*this); }
    virtual void init() override;
    virtual const std::string getAsJson() const override;
};

class OrderFillEvent : public OrderEventBase {
public:
    OrderFillEvent();
    OrderFillEvent(const OrderFillEvent& event);
    OrderFillEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const uint32_t fillQuantity, const double fillPrice);
    const uint32_t getFillQuantity() const { return myFillQuantity; }
    const double getFillPrice() const { return myFillPrice; }
    void setFillQuantity(const uint32_t fillQuantity) { myFillQuantity = fillQuantity; }
    void setFillPrice(const double fillPrice) { myFillPrice = fillPrice; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderFillEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJson() const override;
private:
    uint32_t myFillQuantity;
    double myFillPrice;
};

class OrderModifyPriceEvent : public OrderEventBase {
public:
    OrderModifyPriceEvent();
    OrderModifyPriceEvent(const OrderModifyPriceEvent& event);
    OrderModifyPriceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedPrice);
    const double getModifiedPrice() const { return myModifiedPrice; }
    void setModifiedPrice(const double modifiedPrice) { myModifiedPrice = modifiedPrice; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyPriceEvent>(*this); }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJson() const override;
private:
    double myModifiedPrice;
};

class OrderModifyQuantityEvent : public OrderEventBase {
public:
    OrderModifyQuantityEvent();
    OrderModifyQuantityEvent(const OrderModifyQuantityEvent& event);
    OrderModifyQuantityEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity);
    const uint32_t getModifiedQuantity() const { return myModifiedQuantity; }
    void setModifiedQuantity(const uint32_t modifiedQuantity) { myModifiedQuantity = modifiedQuantity; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyQuantityEvent>(*this); }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual const std::string getAsJson() const override;
private:
    uint32_t myModifiedQuantity;
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
