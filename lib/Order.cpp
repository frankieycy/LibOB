#ifndef ORDER_CPP
#define ORDER_CPP
#include "Utils.hpp"
#include "MetaInfo.hpp"
#include "OrderUtils.hpp"
#include "OrderEvent.hpp"
#include "Order.hpp"

namespace Market {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const OrderBase& order) {
    out << order.getAsJason();
    return out;
}

OrderBase::OrderBase() :
    myId(0),
    myTimestamp(0),
    mySide(Side::NULL_SIDE),
    myQuantity(0),
    myOrderType(OrderType::NULL_ORDER_TYPE),
    myOrderState(OrderState::NULL_ORDER_STATE) {}

OrderBase::OrderBase(const uint64_t id, const uint64_t timestamp, const Side side, const int quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo) :
    myId(id),
    myTimestamp(timestamp),
    mySide(side),
    myQuantity(quantity),
    myOrderType(OrderType::NULL_ORDER_TYPE),
    myOrderState(OrderState::NULL_ORDER_STATE),
    myMetaInfo(metaInfo) {}

OrderBase::OrderBase(const OrderBase& order) :
    myId(order.myId),
    myTimestamp(order.myTimestamp),
    mySide(order.mySide),
    myQuantity(order.myQuantity),
    myOrderType(order.myOrderType),
    myOrderState(order.myOrderState),
    myMetaInfo(order.myMetaInfo) {}

void OrderBase::init() {
    if (myQuantity < 0)
        Error::LIB_THROW("OrderBase: quantity cannot be negative.");
}

void OrderBase::cancel() {
    mySide = Side::NULL_SIDE;
    myQuantity = 0;
    myOrderState = OrderState::CANCELLED;
}

const std::string OrderBase::getAsJason() const {
    std::ostringstream oss;
    oss << "{"
    "\"Id\":"           << getId()         << ","
    "\"Timestamp\":"    << getTimestamp()  << ","
    "\"Side\":\""       << getSide()       << "\","
    "\"Quantity\":"     << getQuantity()   << ","
    "\"OrderType\":\""  << getOrderType()  << "\","
    "\"OrderState\":\"" << getOrderState() << "\"";
    if (getMetaInfo())
        oss << ",\"MetaInfo\":" << *getMetaInfo();
    else
        oss << ",\"MetaInfo\":" << "{}";
    oss << "}";
    return oss.str();
}

LimitOrder::LimitOrder() :
    OrderBase(),
    myPrice(0) {
    init();
}

LimitOrder::LimitOrder(const uint64_t id, const uint64_t timestamp, const Side side, const int quantity, const double price, const std::shared_ptr<OrderMetaInfo>& metaInfo) :
    OrderBase(id, timestamp, side, quantity, metaInfo),
    myPrice(price) {
    init();
}

LimitOrder::LimitOrder(const LimitOrder& order) :
    OrderBase(order),
    myPrice(order.myPrice) {}

void LimitOrder::executeOrderEvent(const OrderEventBase& event) {
    event.applyTo(*this);
}

void LimitOrder::init() {
    if (myPrice < 0)
        Error::LIB_THROW("LimitBase: price cannot be negative.");
    setOrderType(OrderType::LIMIT);
}

void LimitOrder::cancel() {
    OrderBase::cancel();
    myPrice = isBuy() ? 0 : Consts::POS_INF_DOUBLE;
}

const std::string LimitOrder::getAsJason() const {
    std::ostringstream oss;
    oss << "{"
    "\"Id\":"           << getId()         << ","
    "\"Timestamp\":"    << getTimestamp()  << ","
    "\"Side\":\""       << getSide()       << "\","
    "\"Quantity\":"     << getQuantity()   << ","
    "\"Price\":"        << getPrice()      << ","
    "\"OrderType\":\""  << getOrderType()  << "\","
    "\"OrderState\":\"" << getOrderState() << "\"";
    if (getMetaInfo())
        oss << ",\"MetaInfo\":" << *getMetaInfo();
    else
        oss << ",\"MetaInfo\":" << "{}";
    oss << "}";
    return oss.str();
}

MarketOrder::MarketOrder() :
    OrderBase() {
    init();
}

MarketOrder::MarketOrder(const uint64_t id, const uint64_t timestamp, const Side side, const int quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo) :
    OrderBase(id, timestamp, side, quantity, metaInfo) {
    init();
}

MarketOrder::MarketOrder(const MarketOrder& order) :
    OrderBase(order) {
    init();
}

void MarketOrder::executeOrderEvent(const OrderEventBase& event) {
    event.applyTo(*this);
}

void MarketOrder::init() {
    setOrderType(OrderType::MARKET);
}
}

#endif
