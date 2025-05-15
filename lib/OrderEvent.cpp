#ifndef ORDER_EVENT_CPP
#define ORDER_EVENT_CPP
#include "Utils.hpp"
#include "OrderUtils.hpp"
#include "OrderEvent.hpp"
#include "Order.hpp"

namespace Market {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const OrderEventBase& event) {
    out << event.getAsJson();
    return out;
}

OrderEventBase::OrderEventBase() :
    myEventId(0),
    myOrderId(0),
    myTimestamp(0),
    myEventType(OrderEventType::NULL_ORDER_EVENT_TYPE) {}

OrderEventBase::OrderEventBase(const OrderEventBase& event) :
    myEventId(event.myEventId),
    myOrderId(event.myOrderId),
    myTimestamp(event.myTimestamp),
    myEventType(event.myEventType),
    myOrder(event.myOrder) {}

OrderEventBase::OrderEventBase(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<OrderBase>& order) :
    myEventId(eventId),
    myOrderId(orderId),
    myTimestamp(timestamp),
    myOrder(order),
    myEventType(OrderEventType::NULL_ORDER_EVENT_TYPE) {}

void OrderEventBase::applyTo(MarketOrder& order) const {
    Error::LIB_THROW("No implementation for OrderEventBase::applyTo(MarketOrder&).");
}

void OrderEventBase::applyTo(LimitOrder& order) const {
    Error::LIB_THROW("No implementation for OrderEventBase::applyTo(LimitOrder&).");
}

const std::string OrderEventBase::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << getEventId()   << ","
    "\"OrderId\":"     << getOrderId()   << ","
    "\"Timestamp\":"   << getTimestamp() << ","
    "\"EventType\":\"" << getEventType() << "\"";
    oss << "}";
    return oss.str();
}

OrderSubmitEvent::OrderSubmitEvent() :
    OrderEventBase() {
    init();
}

OrderSubmitEvent::OrderSubmitEvent(const OrderSubmitEvent& event) :
    OrderEventBase(event) {
    init();
}

OrderSubmitEvent::OrderSubmitEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const std::shared_ptr<OrderBase>& order) :
    OrderEventBase(eventId, orderId, timestamp, order) {
    init();
}

void OrderSubmitEvent::init() {
    if (!getOrder())
        Error::LIB_THROW("OrderSubmitEvent: order is null.");
    setEventType(OrderEventType::SUBMIT);
}

const std::string OrderSubmitEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << getEventId()   << ","
    "\"OrderId\":"     << getOrderId()   << ","
    "\"Timestamp\":"   << getTimestamp() << ","
    "\"EventType\":\"" << getEventType() << "\","
    "\"Order\":"       << getOrder()->getAsJson();
    oss << "}";
    return oss.str();
}

OrderFillEvent::OrderFillEvent() :
    OrderEventBase(),
    myFillQuantity(0),
    myFillPrice(0) {
    init();
}

OrderFillEvent::OrderFillEvent(const OrderFillEvent& event) :
    OrderEventBase(event),
    myFillQuantity(event.myFillQuantity),
    myFillPrice(event.myFillPrice) {
    init();
}

OrderFillEvent::OrderFillEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const uint32_t fillQuantity, const double fillPrice) :
    OrderEventBase(eventId, orderId, timestamp),
    myFillQuantity(0),
    myFillPrice(0) {
    init();
}

void OrderFillEvent::applyTo(MarketOrder& order) const {
    order.setQuantity(0);
    order.setOrderState(OrderState::FILLED);
}

void OrderFillEvent::applyTo(LimitOrder& order) const {
    order.setQuantity(0);
    order.setOrderState(OrderState::FILLED);
}

void OrderFillEvent::init() {
    if (myFillPrice < 0)
        Error::LIB_THROW("OrderFillEvent: price cannot be negative.");
    setEventType(OrderEventType::FILL);
}

const std::string OrderFillEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"      << getEventId()      << ","
    "\"OrderId\":"      << getOrderId()      << ","
    "\"Timestamp\":"    << getTimestamp()    << ","
    "\"EventType\":\""  << getEventType()    << "\","
    "\"FillQuantity\":" << getFillQuantity() << ","
    "\"FillPrice\":"    << getFillPrice();
    oss << "}";
    return oss.str();
}

OrderModifyPriceEvent::OrderModifyPriceEvent() :
    OrderEventBase(),
    myModifiedPrice(0) {
    init();
}

OrderModifyPriceEvent::OrderModifyPriceEvent(const OrderModifyPriceEvent& event) :
    OrderEventBase(event),
    myModifiedPrice(event.myModifiedPrice) {
    init();
}

OrderModifyPriceEvent::OrderModifyPriceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedPrice) :
    OrderEventBase(eventId, orderId, timestamp),
    myModifiedPrice(modifiedPrice) {
    init();
}

void OrderModifyPriceEvent::applyTo(LimitOrder& order) const {
    order.setPrice(myModifiedPrice);
}

void OrderModifyPriceEvent::init() {
    if (myModifiedPrice < 0)
        Error::LIB_THROW("OrderModifyPriceEvent: price cannot be negative.");
    setEventType(OrderEventType::MODIFY_PRICE);
}

const std::string OrderModifyPriceEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"       << getEventId()      << ","
    "\"OrderId\":"       << getOrderId()      << ","
    "\"Timestamp\":"     << getTimestamp()    << ","
    "\"EventType\":\""   << getEventType()    << "\","
    "\"ModifiedPrice\":" << getModifiedPrice();
    oss << "}";
    return oss.str();
}

OrderModifyQuantityEvent::OrderModifyQuantityEvent() :
    OrderEventBase(),
    myModifiedQuantity(0) {}

OrderModifyQuantityEvent::OrderModifyQuantityEvent(const OrderModifyQuantityEvent& event) :
    OrderEventBase(event),
    myModifiedQuantity(event.myModifiedQuantity) {}

OrderModifyQuantityEvent::OrderModifyQuantityEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity) :
    OrderEventBase(eventId, orderId, timestamp),
    myModifiedQuantity(modifiedQuantity) {}

void OrderModifyQuantityEvent::applyTo(LimitOrder& order) const {
    order.setQuantity(myModifiedQuantity);
}

void OrderModifyQuantityEvent::init() {
    setEventType(OrderEventType::MODIFY_QUANTITY);
}

const std::string OrderModifyQuantityEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"       << getEventId()      << ","
    "\"OrderId\":"       << getOrderId()      << ","
    "\"Timestamp\":"     << getTimestamp()    << ","
    "\"EventType\":\""   << getEventType()    << "\","
    "\"ModifiedQuantity\":" << getModifiedQuantity();
    oss << "}";
    return oss.str();
}

OrderCancelEvent::OrderCancelEvent() :
    OrderEventBase() {
    init();
}

OrderCancelEvent::OrderCancelEvent(const OrderCancelEvent& event) :
    OrderEventBase(event) {
    init();
}

OrderCancelEvent::OrderCancelEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp) :
    OrderEventBase(eventId, orderId, timestamp) {
    init();
}

void OrderCancelEvent::applyTo(MarketOrder& order) const {
    order.setQuantity(0);
    order.setOrderState(OrderState::CANCELLED);
}

void OrderCancelEvent::applyTo(LimitOrder& order) const {
    order.setQuantity(0);
    order.setOrderState(OrderState::CANCELLED);
}

void OrderCancelEvent::init() {
    setEventType(OrderEventType::CANCEL);
}
}

#endif
