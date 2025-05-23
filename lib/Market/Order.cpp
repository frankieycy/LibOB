#ifndef ORDER_CPP
#define ORDER_CPP
#include "Utils/Utils.hpp"
#include "Market/MetaInfo.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEvent.hpp"
#include "Market/Order.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Market {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const OrderBase& order) {
    return out << order.getAsJson();
}

OrderBase::OrderBase() :
    myId(0),
    myTimestamp(0),
    mySide(Side::NULL_SIDE),
    myQuantity(0),
    myOrderType(OrderType::NULL_ORDER_TYPE),
    myOrderState(OrderState::NULL_ORDER_STATE) {}

OrderBase::OrderBase(const OrderBase& order) :
    myId(order.myId),
    myTimestamp(order.myTimestamp),
    mySide(order.mySide),
    myQuantity(order.myQuantity),
    myOrderType(order.myOrderType),
    myOrderState(order.myOrderState),
    myMetaInfo(order.myMetaInfo) {}

OrderBase::OrderBase(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<const OrderMetaInfo>& metaInfo) :
    myId(id),
    myTimestamp(timestamp),
    mySide(side),
    myQuantity(quantity),
    myOrderType(OrderType::NULL_ORDER_TYPE),
    myOrderState(OrderState::NULL_ORDER_STATE),
    myMetaInfo(metaInfo) {}

bool OrderBase::checkState() const {
    switch (myOrderState) {
        case OrderState::ACTIVE:
            return myQuantity > 0;
        case OrderState::PARTIAL_FILLED:
            return myQuantity > 0;
        case OrderState::FILLED:
            return myQuantity == 0;
        case OrderState::CANCELLED:
            return myQuantity == 0;
        default:
            return false;
    }
}

void OrderBase::init() {
    setOrderState(OrderState::ACTIVE);
}

void OrderBase::cancel() {
    mySide = Side::NULL_SIDE;
    myQuantity = 0;
    myOrderState = OrderState::CANCELLED;
}

std::string OrderBase::getAsJson() const {
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

LimitOrder::LimitOrder(const LimitOrder& order) :
    OrderBase(order),
    myPrice(order.myPrice) {}

LimitOrder::LimitOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const double price, const std::shared_ptr<OrderMetaInfo>& metaInfo) :
    OrderBase(id, timestamp, side, quantity, metaInfo),
    myPrice(price) {
    init();
}

void LimitOrder::executeOrderEvent(const OrderEventBase& event) {
    event.applyTo(*this);
}

void LimitOrder::submit(Exchange::IMatchingEngine& matchingEngine) const {
    matchingEngine.addToLimitOrderBook(std::make_shared<LimitOrder>(*this));
}

bool LimitOrder::checkState() const {
    return (myPrice >= 0) && OrderBase::checkState();
}

void LimitOrder::init() {
    if (getSide() == Side::NULL_SIDE)
        Error::LIB_THROW("[LimitOrder::init] Side cannot be null.");
    if (myPrice < 0)
        Error::LIB_THROW("[LimitOrder::init] Price cannot be negative.");
    setOrderState(OrderState::ACTIVE);
    setOrderType(OrderType::LIMIT);
}

void LimitOrder::cancel() {
    OrderBase::cancel();
    myPrice = isBuy() ? 0 : Consts::POS_INF_DOUBLE;
}

std::string LimitOrder::getAsJson() const {
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

MarketOrder::MarketOrder(const MarketOrder& order) :
    OrderBase(order) {
    init();
}

MarketOrder::MarketOrder(const uint64_t id, const uint64_t timestamp, const Side side, const uint32_t quantity, const std::shared_ptr<OrderMetaInfo>& metaInfo) :
    OrderBase(id, timestamp, side, quantity, metaInfo) {
    init();
}

void MarketOrder::executeOrderEvent(const OrderEventBase& event) {
    event.applyTo(*this);
}

void MarketOrder::submit(Exchange::IMatchingEngine& matchingEngine) const {
    matchingEngine.executeMarketOrder(std::make_shared<MarketOrder>(*this));
}

void MarketOrder::init() {
    if (getSide() == Side::NULL_SIDE)
        Error::LIB_THROW("[MarketOrder::init] Side cannot be null.");
    setOrderState(OrderState::ACTIVE);
    setOrderType(OrderType::MARKET);
}
}

#endif
