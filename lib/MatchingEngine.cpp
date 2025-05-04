#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
void IMatchingEngine::process(const std::shared_ptr<Market::OrderBase>& order) {
    order->submit(*this);
}

std::ostream& IMatchingEngine::orderBookSnapshot(std::ostream& out) const {
    return out;
}

const std::string IMatchingEngine::getAsJason() const {
    return "";
}
}

#endif
