#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
void IMatchingEngine::process(const std::shared_ptr<Market::OrderBase>& order) {
    order->submit(*this);
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderEventBase>& event) {
    //
}

std::ostream& IMatchingEngine::orderBookSnapshot(std::ostream& out) const {
    return out;
}

const std::string IMatchingEngine::getAsJason() const {
    return "";
}

void MatchingEngineFIFO::addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) {
    //
}

void MatchingEngineFIFO::executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) {
    //
}

void MatchingEngineFIFO::init() {
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
