#ifndef ZERO_INTELLIGENCE_UTILS_CPP
#define ZERO_INTELLIGENCE_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ZeroIntelligenceUtils.hpp"

namespace Simulator {
double OrderEventRateSamplerProportionalTotalSizeFromOppositeBest::sample() const {
    const auto& bookSnapshot = getLastOrderBookTopLevelsSnapshot();
    const double priceTick = getPriceTick();
    uint32_t totalSize = 0;
    if (!bookSnapshot.askBookTopPrices.empty()) {
        const double bestAskPrice = bookSnapshot.askBookTopPrices[0];
        const double minPrice = bestAskPrice - myMaxOffsetTicks * priceTick;
        const double maxPrice = bestAskPrice - myMinOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.bidBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.bidBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                totalSize += bookSnapshot.bidBookTopSizes[i];
        }
    }
    if (!bookSnapshot.bidBookTopPrices.empty()) {
        const double bestBidPrice = bookSnapshot.bidBookTopPrices[0];
        const double minPrice = bestBidPrice + myMinOffsetTicks * priceTick;
        const double maxPrice = bestBidPrice + myMaxOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.askBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.askBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                totalSize += bookSnapshot.askBookTopSizes[i];
        }
    }
    return myRate * static_cast<double>(totalSize);
}

Market::Side OrderSideSamplerProportionalTotalSizeFromOppositeBest::sample() const {
    const auto& bookSnapshot = getLastOrderBookTopLevelsSnapshot();
    const double priceTick = getPriceTick();
    uint32_t totalBuySize = 0;
    uint32_t totalSellSize = 0;
    if (!bookSnapshot.askBookTopPrices.empty()) {
        const double bestAskPrice = bookSnapshot.askBookTopPrices[0];
        const double minPrice = bestAskPrice - myMaxOffsetTicks * priceTick;
        const double maxPrice = bestAskPrice - myMinOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.bidBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.bidBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                totalBuySize += bookSnapshot.bidBookTopSizes[i];
        }
    }
    if (!bookSnapshot.bidBookTopPrices.empty()) {
        const double bestBidPrice = bookSnapshot.bidBookTopPrices[0];
        const double minPrice = bestBidPrice + myMinOffsetTicks * priceTick;
        const double maxPrice = bestBidPrice + myMaxOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.askBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.askBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                totalSellSize += bookSnapshot.askBookTopSizes[i];
        }
    }
    const double totalSize = static_cast<double>(totalBuySize + totalSellSize);
    if (Utils::Consts::isZero(totalSize))
        return Market::Side::NONE;
    const double buyProbability = static_cast<double>(totalBuySize) / totalSize;
    return Utils::Statistics::getRandomUniform01(true) < buyProbability ? Market::Side::BUY : Market::Side::SELL;
}

double OrderPricePlacementSamplerUniformFromOppositeBest::sample(const Market::Side side) const {
    const auto& bookSnapshot = getLastOrderBookTopLevelsSnapshot();
    const double priceTick = getPriceTick();
    double minPrice = Utils::Consts::NAN_DOUBLE;
    double maxPrice = Utils::Consts::NAN_DOUBLE;
    if (side == Market::Side::BUY) {
        if (bookSnapshot.askBookTopPrices.empty())
            return Utils::Consts::NAN_DOUBLE;
        const double bestAskPrice = bookSnapshot.askBookTopPrices[0];
        minPrice = bestAskPrice - myMaxOffsetTicks * priceTick;
        maxPrice = bestAskPrice - myMinOffsetTicks * priceTick;
    } else if (side == Market::Side::SELL) {
        if (bookSnapshot.bidBookTopPrices.empty())
            return Utils::Consts::NAN_DOUBLE;
        const double bestBidPrice = bookSnapshot.bidBookTopPrices[0];
        minPrice = bestBidPrice + myMinOffsetTicks * priceTick;
        maxPrice = bestBidPrice + myMaxOffsetTicks * priceTick;
    } else {
        return Utils::Consts::NAN_DOUBLE;
    }
    const double sampledPrice = Utils::Statistics::getRandomUniform(minPrice, maxPrice, true);
    return Utils::Maths::roundPriceToTick(sampledPrice, priceTick);
}

std::optional<OrderCancelSpec> OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest::sample(const Market::Side side) const {
    const auto& bookSnapshot = getLastOrderBookTopLevelsSnapshot();
    const double priceTick = getPriceTick();
    double minPrice = Utils::Consts::NAN_DOUBLE;
    double maxPrice = Utils::Consts::NAN_DOUBLE;
    std::vector<double> samplePrices;
    if (side == Market::Side::BUY) {
        if (bookSnapshot.askBookTopPrices.empty())
            return std::nullopt;
        const double bestAskPrice = bookSnapshot.askBookTopPrices[0];
        minPrice = bestAskPrice - myMaxOffsetTicks * priceTick;
        maxPrice = bestAskPrice - myMinOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.bidBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.bidBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                samplePrices.push_back(price);
        }
    } else if (side == Market::Side::SELL) {
        if (bookSnapshot.bidBookTopPrices.empty())
            return std::nullopt;
        const double bestBidPrice = bookSnapshot.bidBookTopPrices[0];
        minPrice = bestBidPrice + myMinOffsetTicks * priceTick;
        maxPrice = bestBidPrice + myMaxOffsetTicks * priceTick;
        for (size_t i = 0; i < bookSnapshot.askBookTopPrices.size(); ++i) {
            const double price = bookSnapshot.askBookTopPrices[i];
            if (price >= minPrice && price <= maxPrice)
                samplePrices.push_back(price);
        }
    } else {
        return std::nullopt;
    }
    if (samplePrices.empty())
        return std::nullopt;
    const size_t randomIndex = Utils::Statistics::getRandomUniformInt<size_t>(0, samplePrices.size() - 1, true);
    return OrderCancelSpec{ mySize, Utils::Maths::roundPriceToTick(samplePrices[randomIndex], priceTick) };
}
}

#endif
