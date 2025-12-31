#ifndef ZERO_INTELLIGENCE_UTILS_CPP
#define ZERO_INTELLIGENCE_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ZeroIntelligenceUtils.hpp"

namespace Simulator {
using namespace Utils;

double UniformOrderPricePlacementFromOppositeBestSampler::sample(const Market::Side side) const {
    const auto& bookSnapshot = getLastOrderBookTopLevelsSnapshot();
    const double priceTick = getPriceTick();
    double minPrice = Consts::NAN_DOUBLE;
    double maxPrice = Consts::NAN_DOUBLE;
    if (side == Market::Side::BUY) {
        if (bookSnapshot.askBookTopPrices.empty())
            Error::LIB_THROW("[UniformOrderPricePlacementFromOppositeBestSampler] Cannot sample buy order price placement from empty ask book.");
        const double bestAskPrice = bookSnapshot.askBookTopPrices[0];
        minPrice = bestAskPrice - myMaxOffsetTicks * priceTick;
        maxPrice = bestAskPrice - myMinOffsetTicks * priceTick;
    } else if (side == Market::Side::SELL) {
        if (bookSnapshot.bidBookTopPrices.empty())
            Error::LIB_THROW("[UniformOrderPricePlacementFromOppositeBestSampler] Cannot sample sell order price placement from empty bid book.");
        const double bestBidPrice = bookSnapshot.bidBookTopPrices[0];
        minPrice = bestBidPrice + myMinOffsetTicks * priceTick;
        maxPrice = bestBidPrice + myMaxOffsetTicks * priceTick;
    } else {
        Error::LIB_THROW("[UniformOrderPricePlacementFromOppositeBestSampler] Invalid order side for price placement sampling.");
    }
    const double sampledPrice = Statistics::getRandomUniform(minPrice, maxPrice, true);
    return Maths::roundPriceToTick(sampledPrice, priceTick);
}
}

#endif
