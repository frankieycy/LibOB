#ifndef TRADE_CPP
#define TRADE_CPP
#include "Utils.hpp"
#include "MetaInfo.hpp"
#include "Order.hpp"
#include "Trade.hpp"

namespace Market {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const TradeBase& trade) {
    out << trade.getAsJson();
    return out;
}

TradeBase::TradeBase() :
    myId(0),
    myTimestamp(0),
    myBuyId(0),
    mySellId(0),
    myQuantity(0),
    myPrice(0),
    myIsBuyLimitOrder(true),
    myIsSellLimitOrder(true),
    myIsBuyInitiated(true) {}

TradeBase::TradeBase(const TradeBase& trade) :
    myId(trade.myId),
    myTimestamp(trade.myTimestamp),
    myBuyId(trade.myBuyId),
    mySellId(trade.mySellId),
    myQuantity(trade.myQuantity),
    myPrice(trade.myPrice),
    myIsBuyLimitOrder(trade.myIsBuyLimitOrder),
    myIsSellLimitOrder(trade.myIsSellLimitOrder),
    myIsBuyInitiated(trade.myIsBuyInitiated),
    myMetaInfo(trade.myMetaInfo) {
    init();
}

TradeBase::TradeBase(
    const uint64_t id, const uint64_t timestamp, const uint64_t buyId, const uint64_t sellId, const uint32_t quantity, const double price,
    const bool isBuyLimitOrder, const bool isSellLimitOrder, const bool isBuyInitiated, const std::shared_ptr<const TradeMetaInfo>& metaInfo) :
    myId(id),
    myTimestamp(timestamp),
    myBuyId(buyId),
    mySellId(sellId),
    myQuantity(quantity),
    myPrice(price),
    myIsBuyLimitOrder(isBuyLimitOrder),
    myIsSellLimitOrder(isSellLimitOrder),
    myIsBuyInitiated(isBuyInitiated),
    myMetaInfo(metaInfo) {
    init();
}

TradeBase::TradeBase(const uint64_t id, const uint64_t timestamp, const uint32_t quantity, const double price, const bool isBuyInitiated, const OrderBase& buyOrder, const OrderBase& sellOrder) :
    myId(id),
    myTimestamp(timestamp),
    myBuyId(buyOrder.getId()),
    mySellId(sellOrder.getId()),
    myQuantity(quantity),
    myPrice(price),
    myIsBuyLimitOrder(buyOrder.getOrderType() == OrderType::LIMIT),
    myIsSellLimitOrder(sellOrder.getOrderType() == OrderType::LIMIT),
    myIsBuyInitiated(isBuyInitiated),
    myMetaInfo(isBuyInitiated ? buyOrder.getMetaInfo() : sellOrder.getMetaInfo()) {
    init();
}

void TradeBase::init() {
    if (myPrice < 0)
        Error::LIB_THROW("TradeBase: price cannot be negative.");
}

std::string TradeBase::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"Id\":"               << getId()               << ","
    "\"Timestamp\":"        << getTimestamp()        << ","
    "\"BuyId\":"            << getBuyId()            << ","
    "\"SellId\":"           << getSellId()           << ","
    "\"Quantity\":"         << getQuantity()         << ","
    "\"Price\":"            << getPrice()            << ","
    "\"IsBuyLimitOrder\":"  << getIsBuyLimitOrder()  << ","
    "\"IsSellLimitOrder\":" << getIsSellLimitOrder() << ","
    "\"IsBuyInitiated\":"   << getIsBuyInitiated()   << ","
    "\"Symbol\":\""         << getSymbol()           << "\","
    "\"ExchangeId\":\""     << getExchangeId()       << "\"";
    oss << "}";
    return oss.str();
}
}

#endif
