#ifndef ORDER_BOOK_DERIVED_ANALYTICS_CPP
#define ORDER_BOOK_DERIVED_ANALYTICS_CPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"
#include "Analytics/OrderBookDerivedAnalyticsUtils.hpp"

namespace Analytics {
using namespace Utils;

void OrderFlowMemoryStats::accumulate(const int8_t tradeSign) {
    tradeSignsACF.add(tradeSign);
}

void OrderFlowMemoryStats::reset() {
    tradeSignsACF.clear();
}

void OrderDepthProfileStats::set(const OrderDepthProfileConfig& config) {
    normalization = config.normalization;
    priceSpace = config.priceSpace;
    countMissingLevels = config.countMissingLevels;
}

void OrderDepthProfileStats::init() {
    if (normalization == DepthNormalization::NONE)
        Error::LIB_THROW("[OrderDepthProfileStats::init] Depth normalization is NONE.");
    if (priceSpace == PriceSpaceDefinition::NONE)
        Error::LIB_THROW("[OrderDepthProfileStats::init] Price space definition is NONE.");
    if (!avgBid.empty() || !avgAsk.empty() || !stdBid.empty() || !stdAsk.empty() ||
        !sumBid.empty() || !sumAsk.empty() || !sumSqBid.empty() || !sumSqAsk.empty() ||
        !nonZeroCountBid.empty() || !nonZeroCountAsk.empty())
        Error::LIB_THROW("[OrderDepthProfileStats::init] Stats vectors are not empty during initialization. Clear them via reset() first.");
}

void OrderDepthProfileStats::reset() {
    numSnapshots = 0;
    maxTicks = 0;
    avgBid.clear();
    avgAsk.clear();
    stdBid.clear();
    stdAsk.clear();
    nonZeroCountBid.clear();
    nonZeroCountAsk.clear();
    sumBid.clear();
    sumAsk.clear();
    sumSqBid.clear();
    sumSqAsk.clear();
}

void SpreadStats::accumulate(const double spread) {
    spreadHistogram.add(spread);
    spreadACF.add(spread);
}

void SpreadStats::reset() {
    spreadHistogram.clear();
    spreadACF.clear();
}

void PriceReturnScalingStats::set(const PriceReturnScalingStatsConfig& config) {
    priceType = config.priceType;
    logReturns = config.logReturns;
}

void PriceReturnScalingStats::init() {
    if (priceType == PriceType::NONE)
        Error::LIB_THROW("[PriceReturnScalingStats::init] Price type is NONE.");
    if (!horizons.empty() || !sumReturns.empty() || !sumSqReturns.empty() || !counts.empty())
        Error::LIB_THROW("[PriceReturnScalingStats::init] Stats vectors are not empty during initialization. Clear them via reset() first.");
}

void PriceReturnScalingStats::reset() {
    horizons.clear();
    sumReturns.clear();
    sumSqReturns.clear();
    counts.clear();
}
}

#endif
