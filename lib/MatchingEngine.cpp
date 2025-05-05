#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine) {
    out << matchingEngine.getAsJason();
    return out;
}

const std::pair<const PriceLevel, int> IMatchingEngine::getBestBidPriceAndSize() const {
    if (myBidBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myBidBookSize.begin();
}

const std::pair<const PriceLevel, int> IMatchingEngine::getBestAskPriceAndSize() const {
    if (myAskBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myAskBookSize.begin();
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestBidTopOrder() const {
    if (myBidBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myBidBook.begin();
    return {it->first, it->second.front()};
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestAskTopOrder() const {
    if (myAskBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myAskBook.begin();
    return {it->first, it->second.front()};
}

const double IMatchingEngine::getBestBidPrice() const {
    if (myBidBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myBidBookSize.begin()->first;
}

const double IMatchingEngine::getBestAskPrice() const {
    if (myAskBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myAskBookSize.begin()->first;
}

const double IMatchingEngine::getSpread() const {
    return getBestAskPrice() - getBestBidPrice();
}

const double IMatchingEngine::getHalfSpread() const {
    return getSpread() / 2.0;
}

const double IMatchingEngine::getMidPrice() const {
    return (getBestBidPrice() + getBestAskPrice()) / 2.0;
}

const double IMatchingEngine::getMicroPrice() const {
    return (getBestBidPrice() * getBestAskSize() + getBestAskPrice() * getBestBidSize()) / (getBestBidSize() + getBestAskSize());
}

const double IMatchingEngine::getOrderImbalance() const {
    return (getBestBidSize() - getBestAskSize()) / (getBestBidSize() + getBestAskSize());
}

const double IMatchingEngine::getLastTradePrice() const {
    return getLastTrade()->getPrice();
}

const uint32_t IMatchingEngine::getBestBidSize() const {
    if (myBidBookSize.empty())
        return 0;
    return myBidBookSize.begin()->second;
}

const uint32_t IMatchingEngine::getBestAskSize() const {
    if (myAskBookSize.empty())
        return 0;
    return myAskBookSize.begin()->second;
}

const uint32_t IMatchingEngine::getBidSize(const PriceLevel& priceLevel) const {
    const auto& it = myBidBookSize.find(priceLevel);
    return it != myBidBookSize.end() ? it->second : 0;
}

const uint32_t IMatchingEngine::getAskSize(const PriceLevel& priceLevel) const {
    const auto& it = myAskBookSize.find(priceLevel);
    return it != myAskBookSize.end() ? it->second : 0;
}

const uint32_t IMatchingEngine::getLastTradeSize() const {
    return getLastTrade()->getQuantity();
}

const size_t IMatchingEngine::getNumberOfBidPriceLevels() const {
    return myBidBook.size();
}

const size_t IMatchingEngine::getNumberOfAskPriceLevels() const {
    return myAskBook.size();
}

const size_t IMatchingEngine::getNumberOfTrades() const {
    return myTradeLog.size();
}

const std::shared_ptr<Market::TradeBase> IMatchingEngine::getLastTrade() const {
    if (myTradeLog.empty())
        return nullptr;
    return myTradeLog.back();
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderBase>& order) {
    if (!order)
        Error::LIB_THROW("IMatchingEngine::process: order is null.");
    order->submit(*this);
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderEventBase>& event) {
    if (!event)
        Error::LIB_THROW("IMatchingEngine::process: order event is null.");
    if (event->isSubmit())
        process(event->getOrder());
    const auto& it = myLimitOrderLookup.find(event->getOrderId());
    if (it != myLimitOrderLookup.end())
        (*it->second)->executeOrderEvent(*event);
}

std::ostream& IMatchingEngine::orderBookSnapshot(std::ostream& out) const {
    // TODO
    return out;
}

void IMatchingEngine::init() {
    // TODO
}

void IMatchingEngine::reset() {
    mySymbol.clear();
    myExchangeId.clear();
    myBidBook.clear();
    myAskBook.clear();
    myBidBookSize.clear();
    myAskBookSize.clear();
    myMarketQueue.clear();
    myTradeLog.clear();
    myLimitOrderLookup.clear();
}

const std::string IMatchingEngine::getAsJason() const {
    // TODO
    return "";
}

void MatchingEngineFIFO::addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) {
    // TODO
}

void MatchingEngineFIFO::executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) {
    // TODO
}

void MatchingEngineFIFO::init() {
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
