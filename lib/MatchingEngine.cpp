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
        throw Error::LibException("IMatchingEngine: bid book is empty.");
    return *myBidBookSize.begin();
}

const std::pair<const PriceLevel, int> IMatchingEngine::getBestAskPriceAndSize() const {
    if (myAskBookSize.empty())
        throw Error::LibException("IMatchingEngine: ask book is empty.");
    return *myAskBookSize.begin();
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestBidTopOrder() const {
    // TODO
    return std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>>();
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestAskTopOrder() const {
    // TODO
    return std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>>();
}

const double IMatchingEngine::getBestBidPrice() const {
    if (myBidBookSize.empty())
        throw Error::LibException("IMatchingEngine: bid book is empty.");
    return myBidBookSize.begin()->first;
}

const double IMatchingEngine::getBestAskPrice() const {
    if (myAskBookSize.empty())
        throw Error::LibException("IMatchingEngine: ask book is empty.");
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

const int IMatchingEngine::getBestBidSize() const {
    if (myBidBookSize.empty())
        throw Error::LibException("IMatchingEngine: bid book is empty.");
    return myBidBookSize.begin()->second;
}

const int IMatchingEngine::getBestAskSize() const {
    if (myAskBookSize.empty())
        throw Error::LibException("IMatchingEngine: ask book is empty.");
    return myAskBookSize.begin()->second;
}

const int IMatchingEngine::getBidSize(const PriceLevel& priceLevel) const {
    return myBidBookSize.at(priceLevel);
}

const int IMatchingEngine::getAskSize(const PriceLevel& priceLevel) const {
    return myAskBookSize.at(priceLevel);
}

const int IMatchingEngine::getLastTradeSize() const {
    return getLastTrade()->getQuantity();
}

const int IMatchingEngine::getNumberOfBidPriceLevels() const {
    return myBidBook.size();
}

const int IMatchingEngine::getNumberOfAskPriceLevels() const {
    return myAskBook.size();
}

const int IMatchingEngine::getNumberOfTrades() const {
    return myTradeLog.size();
}

const std::shared_ptr<Market::TradeBase>& IMatchingEngine::getLastTrade() const {
    if (myTradeLog.empty())
        throw Error::LibException("IMatchingEngine: trade log is empty.");
    return myTradeLog.back();
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderBase>& order) {
    if (!order)
        throw Error::LibException("IMatchingEngine::process: order is null.");
    order->submit(*this);
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderEventBase>& event) {
    if (!event)
        throw Error::LibException("IMatchingEngine::process: order event is null.");
    const auto& it = myLimitOrderLookup.find(event->getOrderId());
    if (it != myLimitOrderLookup.end()) 
        it->second->executeOrderEvent(*event);
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

void MatchingEngineFIFO::addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) {
    // TODO
}

void MatchingEngineFIFO::executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) {
    // TODO
}

void MatchingEngineFIFO::init() {
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
