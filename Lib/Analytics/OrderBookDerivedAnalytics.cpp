#ifndef ORDER_BOOK_DERIVED_ANALYTICS_CPP
#define ORDER_BOOK_DERIVED_ANALYTICS_CPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

void PriceReturnScalingStats::set(const PriceType priceType, const bool logReturns) {
    this->priceType = priceType;
    this->logReturns = logReturns;
}

void OrderFlowMemoryStats::accumulate(const int8_t tradeSign) {
    tradeSignsACF.add(tradeSign);
}

void OrderFlowMemoryStats::reset() {
    tradeSignsACF.clear();
}

void OrderDepthProfileStats::set(
    const DepthNormalization normalization,
    const PriceSpaceDefinition priceSpace,
    const bool countMissingLevels) {
    this->normalization = normalization;
    this->priceSpace = priceSpace;
    this->countMissingLevels = countMissingLevels;
}

void SpreadStats::accumulate(const double spread) {
    spreadHistogram.add(spread);
    spreadACF.add(spread);
}

void SpreadStats::reset() {
    spreadHistogram.clear();
    spreadACF.clear();
}
}

#endif
