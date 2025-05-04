#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
MatchingEngineBase::MatchingEngineBase(const MatchingEngineBase& matchingEngine) :
    mySymbol(matchingEngine.mySymbol),
    myExchangeId(matchingEngine.myExchangeId),
    myBidLimitOrderBook(matchingEngine.myBidLimitOrderBook),
    myAskLimitOrderBook(matchingEngine.myAskLimitOrderBook),
    myMarketOrderQueue(matchingEngine.myMarketOrderQueue),
    myTradesLog(matchingEngine.myTradesLog) {}

void MatchingEngineBase::process(const std::shared_ptr<Market::OrderBase>& order) {
    order->submit(*this);
}

void MatchingEngineBase::addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) {}

void MatchingEngineBase::executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) {}

void MatchingEngineBase::init() {}

const std::string MatchingEngineBase::getAsJason() const {
    return "";
}
}

#endif
