#ifndef ORDER_EVENT_CPP
#define ORDER_EVENT_CPP
#include "Utils.hpp"
#include "OrderUtils.hpp"
#include "OrderEvent.hpp"

namespace Market {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const OrderEventBase& event) {
    out << event.getAsJason();
    return out;
}

OrderEventBase::OrderEventBase() :
    myEventId(0),
    myOrderId(0),
    myTimestamp(0),
    myEventType(OrderEventType::NULL_ORDER_EVENT_TYPE) {}

OrderEventBase::OrderEventBase(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp) :
    myEventId(eventId),
    myOrderId(orderId),
    myTimestamp(timestamp),
    myEventType(OrderEventType::NULL_ORDER_EVENT_TYPE) {}

OrderEventBase::OrderEventBase(const OrderEventBase& event) :
    myEventId(event.myEventId),
    myOrderId(event.myOrderId),
    myTimestamp(event.myTimestamp),
    myEventType(event.myEventType) {}

const int OrderEventBase::getFillQuantity() const {
    Error::LIB_THROW("No implementation for OrderEventBase::getFillQuantity.");
    return Consts::NAN_INT;
}

const double OrderEventBase::getFillPrice() const {
    Error::LIB_THROW("No implementation for OrderEventBase::getFillPrice.");
    return Consts::NAN_DOUBLE;
}

const int OrderEventBase::getModifiedQuantity() const {
    Error::LIB_THROW("No implementation for OrderEventBase::getModifiedQuantity.");
    return Consts::NAN_INT;
}

const double OrderEventBase::getModifiedPrice() const {
    Error::LIB_THROW("No implementation for OrderEventBase::getModifiedPrice.");
    return Consts::NAN_DOUBLE;
}

const std::string OrderEventBase::getAsJason() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":" << getEventId() << ","
    "\"OrderId\":" << getOrderId() << ","
    "\"Timestamp\":" << getTimestamp() << ","
    "\"EventType\":\"" << getEventType() << "\"";
    oss << "}";
    return oss.str();
}

OrderFillEvent::OrderFillEvent() :
    OrderEventBase(),
    myFillQuantity(0),
    myFillPrice(0) {
    init();
}

OrderFillEvent::OrderFillEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const int fillQuantity, const double fillPrice) :
    OrderEventBase(eventId, orderId, timestamp),
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

void OrderFillEvent::init() {
    if (myFillQuantity < 0)
        Error::LIB_THROW("OrderFillEvent: quantity cannot be negative.");
    if (myFillPrice < 0)
        Error::LIB_THROW("OrderFillEvent: price cannot be negative.");
    setEventType(OrderEventType::FILL);
}

const std::string OrderFillEvent::getAsJason() const {
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

OrderModifyPriceEvent::OrderModifyPriceEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedPrice) :
    OrderEventBase(eventId, orderId, timestamp),
    myModifiedPrice(modifiedPrice) {
    init();
}

OrderModifyPriceEvent::OrderModifyPriceEvent(const OrderModifyPriceEvent& event) :
    OrderEventBase(event),
    myModifiedPrice(event.myModifiedPrice) {
    init();
}

void OrderModifyPriceEvent::init() {
    if (myModifiedPrice < 0)
        Error::LIB_THROW("OrderModifyPriceEvent: price cannot be negative.");
    setEventType(OrderEventType::MODIFY_PRICE);
}

const std::string OrderModifyPriceEvent::getAsJason() const {
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

OrderModifyQuantityEvent::OrderModifyQuantityEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity) :
    OrderEventBase(eventId, orderId, timestamp),
    myModifiedQuantity(modifiedQuantity) {}

OrderModifyQuantityEvent::OrderModifyQuantityEvent(const OrderModifyQuantityEvent& event) :
    OrderEventBase(event),
    myModifiedQuantity(event.myModifiedQuantity) {}

void OrderModifyQuantityEvent::init() {
    if (myModifiedQuantity < 0)
        Error::LIB_THROW("OrderModifyQuantityEvent: Quantity cannot be negative.");
    setEventType(OrderEventType::MODIFY_QUANTITY);
}

const std::string OrderModifyQuantityEvent::getAsJason() const {
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

OrderCancelEvent::OrderCancelEvent(const uint64_t eventId, const uint64_t orderId, const uint64_t timestamp, const double modifiedQuantity) :
    OrderEventBase(eventId, orderId, timestamp) {
    init();
}

OrderCancelEvent::OrderCancelEvent(const OrderCancelEvent& event) :
    OrderEventBase(event) {
    init();
}

void OrderCancelEvent::init() {
    setEventType(OrderEventType::CANCEL);
}
}

#endif
