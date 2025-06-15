#ifndef ORDER_EVENT_HPP
#define ORDER_EVENT_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"

namespace Market {
class OrderBase;
class MarketOrder;
class LimitOrder;

class OrderEventBase {
public:
    OrderEventBase();
    OrderEventBase(const OrderEventBase& event);
    OrderEventBase(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<const OrderBase>& order = nullptr);
    virtual ~OrderEventBase() = default;
    uint64_t getEventId() const { return myEventId; }
    uint64_t getOrderId() const { return myOrderId; }
    uint64_t getTimestamp() const { return myTimestamp; }
    OrderEventType getEventType() const { return myEventType; }
    std::shared_ptr<const OrderBase> getOrder() const { return myOrder; }
    bool isSubmit() const { return myEventType == OrderEventType::SUBMIT; }
    void setEventId(const uint64_t eventId) { myEventId = eventId; }
    void setOrderId(const uint64_t orderId) { myOrderId = orderId; }
    void setTimestamp(const uint64_t timestamp) { myTimestamp = timestamp; }
    void setEventType(const OrderEventType eventType) { myEventType = eventType; }
    void setOrder(const std::shared_ptr<const OrderBase>& order) { myOrder = order; }
    virtual std::shared_ptr<OrderEventBase> clone() const { return std::make_shared<OrderEventBase>(*this); }
    virtual void applyTo(MarketOrder& order) const;
    virtual void applyTo(LimitOrder& order) const;
    virtual void init() {};
    virtual std::string getAsJson() const;
private:
    uint64_t myEventId;
    uint64_t myOrderId;
    uint64_t myTimestamp;
    OrderEventType myEventType;
    std::shared_ptr<const OrderBase> myOrder;
};

class OrderSubmitEvent : public OrderEventBase {
public:
    OrderSubmitEvent();
    OrderSubmitEvent(const OrderSubmitEvent& event);
    OrderSubmitEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<const OrderBase>& order);
    virtual ~OrderSubmitEvent() = default;
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderSubmitEvent>(*this); }
    virtual void init() override;
    virtual std::string getAsJson() const override;
};

/* Note that OrderFillEvent is not a elemental order event that
    acts on an order, not submitted from the order event manager
    but rather, derived from the matching engine during order
    processing. Currently, this is unused anywhere. */
class OrderFillEvent : public OrderEventBase {
public:
    OrderFillEvent();
    OrderFillEvent(const OrderFillEvent& event);
    OrderFillEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const uint32_t fillQuantity, const double fillPrice);
    virtual ~OrderFillEvent() = default;
    uint32_t getFillQuantity() const { return myFillQuantity; }
    double getFillPrice() const { return myFillPrice; }
    void setFillQuantity(const uint32_t fillQuantity) { myFillQuantity = fillQuantity; }
    void setFillPrice(const double fillPrice) { myFillPrice = fillPrice; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderFillEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    uint32_t myFillQuantity;
    double myFillPrice;
};

class OrderModifyPriceEvent : public OrderEventBase {
public:
    OrderModifyPriceEvent();
    OrderModifyPriceEvent(const OrderModifyPriceEvent& event);
    OrderModifyPriceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedPrice);
    virtual ~OrderModifyPriceEvent() = default;
    double getModifiedPrice() const { return myModifiedPrice; }
    void setModifiedPrice(const double modifiedPrice) { myModifiedPrice = modifiedPrice; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyPriceEvent>(*this); }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    double myModifiedPrice;
};

class OrderModifyQuantityEvent : public OrderEventBase {
public:
    OrderModifyQuantityEvent();
    OrderModifyQuantityEvent(const OrderModifyQuantityEvent& event);
    OrderModifyQuantityEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity);
    virtual ~OrderModifyQuantityEvent() = default;
    uint32_t getModifiedQuantity() const { return myModifiedQuantity; }
    void setModifiedQuantity(const uint32_t modifiedQuantity) { myModifiedQuantity = modifiedQuantity; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderModifyQuantityEvent>(*this); }
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    uint32_t myModifiedQuantity;
};

class OrderCancelEvent : public OrderEventBase {
public:
    OrderCancelEvent();
    OrderCancelEvent(const OrderCancelEvent& event);
    OrderCancelEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp);
    virtual ~OrderCancelEvent() = default;
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderCancelEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
};

class OrderPartialCancelEvent : public OrderEventBase {
public:
    OrderPartialCancelEvent();
    OrderPartialCancelEvent(const OrderPartialCancelEvent& event);
    OrderPartialCancelEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const uint32_t cancelQuantity);
    virtual ~OrderPartialCancelEvent() = default;
    uint32_t getCancelQuantity() const { return myCancelQuantity; }
    void setCancelQuantity(const uint32_t cancelQuantity) { myCancelQuantity = cancelQuantity; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderPartialCancelEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    uint32_t myCancelQuantity;
};

class OrderCancelAndReplaceEvent : public OrderEventBase {
public:
    OrderCancelAndReplaceEvent();
    OrderCancelAndReplaceEvent(const OrderCancelAndReplaceEvent& event);
    OrderCancelAndReplaceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const uint64_t newOrderId,
        const std::optional<uint32_t>& modifiedQuantity = std::nullopt,
        const std::optional<double>& modifiedPrice = std::nullopt);
    virtual ~OrderCancelAndReplaceEvent() = default;
    uint64_t getNewOrderId() const { return myNewOrderId; }
    std::optional<uint32_t> getModifiedQuantity() const { return myModifiedQuantity; }
    std::optional<double> getModifiedPrice() const { return myModifiedPrice; }
    void setNewOrderId(const uint64_t newOrderId) { myNewOrderId = newOrderId; }
    void setModifiedQuantity(const std::optional<uint32_t>& modifiedQuantity) { myModifiedQuantity = modifiedQuantity; }
    void setModifiedPrice(const std::optional<double>& modifiedPrice) { myModifiedPrice = modifiedPrice; }
    virtual std::shared_ptr<OrderEventBase> clone() const override { return std::make_shared<OrderCancelAndReplaceEvent>(*this); }
    virtual void applyTo(MarketOrder& order) const override;
    virtual void applyTo(LimitOrder& order) const override;
    virtual void init() override;
    virtual std::string getAsJson() const override;
private:
    uint64_t myNewOrderId;
    std::optional<uint32_t> myModifiedQuantity;
    std::optional<double> myModifiedPrice;
};

std::ostream& operator<<(std::ostream& out, const OrderEventBase& event);
}

#endif
