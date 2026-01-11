#ifndef ORDER_BOOK_DERIVED_ANALYTICS_CPP
#define ORDER_BOOK_DERIVED_ANALYTICS_CPP
#include "Utils/Utils.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"
#include "Analytics/OrderBookDerivedAnalyticsUtils.hpp"

namespace Analytics {
using namespace Utils;

std::string toString(const OrderDepthProfileStats::DepthNormalization& normalization) {
    switch (normalization) {
        case OrderDepthProfileStats::DepthNormalization::BY_TOTAL_DEPTH:  return "ByTotalDepth";
        case OrderDepthProfileStats::DepthNormalization::BY_BEST_LEVEL:   return "ByBestLevel";
        case OrderDepthProfileStats::DepthNormalization::UNNORMALIZED:    return "Unnormalized";
        default:                                                          return "None";
    }
}

std::string toString(const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace) {
    switch (priceSpace) {
        case OrderDepthProfileStats::PriceSpaceDefinition::DIFF_TO_MID:           return "DiffToMid";
        case OrderDepthProfileStats::PriceSpaceDefinition::DIFF_TO_OWN_BEST:      return "DiffToOwnBest";
        case OrderDepthProfileStats::PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST: return "DiffToOppositeBest";
        default:                                                                  return "None";
    }
}

std::string toString(const PriceReturnScalingStats::PriceType& priceType) {
    switch (priceType) {
        case PriceReturnScalingStats::PriceType::LAST_TRADE: return "LastTrade";
        case PriceReturnScalingStats::PriceType::MID:        return "Mid";
        case PriceReturnScalingStats::PriceType::MICRO:      return "Micro";
        default:                                             return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::DepthNormalization& normalization) { return out << toString(normalization); }
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace) { return out << toString(priceSpace); }
std::ostream& operator<<(std::ostream& out, const PriceReturnScalingStats::PriceType& priceType) { return out << toString(priceType); }

void OrderDepthProfileStats::set(const OrderDepthProfileConfig& config) {
    normalization = config.normalization;
    priceSpace = config.priceSpace;
    maxTicks = config.maxTicks;
    minPriceTick = config.minPriceTick;
    countMissingLevels = config.countMissingLevels;
}

void OrderDepthProfileStats::accumulate(const OrderBookTopLevelsSnapshot& snapshot) {
    if (snapshot.numLevels == 0)
        return; // skip empty snapshots
    ++numSnapshots;
    // build the depth profile based on the price space definition
    std::vector<double> bidDepthProfile(maxTicks, 0.0);
    std::vector<double> askDepthProfile(maxTicks, 0.0);
    const long long llMaxTicks = static_cast<long long>(maxTicks);
    double midPrice = Consts::NAN_DOUBLE;
    double bidRefPrice = Consts::NAN_DOUBLE;
    double askRefPrice = Consts::NAN_DOUBLE;
    switch (priceSpace) {
        case PriceSpaceDefinition::DIFF_TO_MID:
            if (!snapshot.bidBookTopPrices.empty() && !snapshot.askBookTopPrices.empty())
                midPrice = 0.5 * (snapshot.bidBookTopPrices[0] + snapshot.askBookTopPrices[0]);
            else
                return; // cannot compute mid price, skip this snapshot
            break;
        case PriceSpaceDefinition::DIFF_TO_OWN_BEST:
            if (!snapshot.bidBookTopPrices.empty())
                bidRefPrice = snapshot.bidBookTopPrices[0];
            if (!snapshot.askBookTopPrices.empty())
                askRefPrice = snapshot.askBookTopPrices[0];
            break;
        case PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST:
            if (!snapshot.askBookTopPrices.empty())
                bidRefPrice = snapshot.askBookTopPrices[0];
            if (!snapshot.bidBookTopPrices.empty())
                askRefPrice = snapshot.bidBookTopPrices[0];
            break;
        default:
            break;
    }
    for (size_t level = 0; level < snapshot.numLevels; ++level) {
        // compute bid depth profile
        if (level < snapshot.bidBookTopPrices.size()) {
            const double price = snapshot.bidBookTopPrices[level];
            double priceDiff = 0.0;
            switch (priceSpace) {
                case PriceSpaceDefinition::DIFF_TO_MID:
                    priceDiff = midPrice - price;
                    break;
                case PriceSpaceDefinition::DIFF_TO_OWN_BEST:
                case PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST:
                    priceDiff = bidRefPrice - price;
                    break;
                default:
                    continue; // invalid price space definition
            }
            const long long tickIndex = Maths::countPriceTicks(priceDiff, minPriceTick);
            if (tickIndex < llMaxTicks) {
                const uint32_t size = snapshot.bidBookTopSizes[level];
                bidDepthProfile[tickIndex] += static_cast<double>(size);
                if (size)
                    ++nonZeroCountBid[tickIndex];
            }
        }
        // compute ask depth profile
        if (level < snapshot.askBookTopPrices.size()) {
            const double price = snapshot.askBookTopPrices[level];
            double priceDiff = 0.0;
            switch (priceSpace) {
                case PriceSpaceDefinition::DIFF_TO_MID:
                    priceDiff = price - midPrice;
                    break;
                case PriceSpaceDefinition::DIFF_TO_OWN_BEST:
                case PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST:
                    priceDiff = price - askRefPrice;
                    break;
                default:
                    continue; // invalid price space definition
            }
            const long long tickIndex = Maths::countPriceTicks(priceDiff, minPriceTick);
            if (tickIndex < llMaxTicks) {
                const uint32_t size = snapshot.askBookTopSizes[level];
                askDepthProfile[tickIndex] += static_cast<double>(size);
                if (size)
                    ++nonZeroCountAsk[tickIndex];
            }
        }
    }
    // accumulate based on the depth normalization
    double bidDepthDenom = 0.0;
    double askDepthDenom = 0.0;
    if (normalization == DepthNormalization::BY_TOTAL_DEPTH) {
        for (const auto& size : bidDepthProfile)
            bidDepthDenom += size;
        for (const auto& size : askDepthProfile)
            askDepthDenom += size;
    } else if (normalization == DepthNormalization::BY_BEST_LEVEL) {
        bidDepthDenom = bidDepthProfile.empty() ? 0.0 : bidDepthProfile[0];
        askDepthDenom = askDepthProfile.empty() ? 0.0 : askDepthProfile[0];
    }
    if (normalization == DepthNormalization::BY_TOTAL_DEPTH || normalization == DepthNormalization::BY_BEST_LEVEL) {
        if (bidDepthDenom == 0.0 || askDepthDenom == 0.0)
            return; // cannot normalize with zero total depth, skip this snapshot
    }
    for (size_t i = 0; i < maxTicks; ++i) {
        double normBidSize = bidDepthProfile[i];
        double normAskSize = askDepthProfile[i];
        if (normalization == DepthNormalization::BY_TOTAL_DEPTH || normalization == DepthNormalization::BY_BEST_LEVEL) {
            normBidSize /= bidDepthDenom;
            normAskSize /= askDepthDenom;
        }
        sumBid[i] += normBidSize;
        sumAsk[i] += normAskSize;
        sumSqBid[i] += normBidSize * normBidSize;
        sumSqAsk[i] += normAskSize * normAskSize;
    }
}

void OrderDepthProfileStats::init() {
    if (normalization == DepthNormalization::NONE)
        Error::LIB_THROW("[OrderDepthProfileStats::init] Depth normalization is NONE.");
    if (priceSpace == PriceSpaceDefinition::NONE)
        Error::LIB_THROW("[OrderDepthProfileStats::init] Price space definition is NONE.");
    numSnapshots = 0;
    avgBid.clear();
    avgAsk.clear();
    stdBid.clear();
    stdAsk.clear();
    sumBid.resize(maxTicks, 0.0);
    sumAsk.resize(maxTicks, 0.0);
    sumSqBid.resize(maxTicks, 0.0);
    sumSqAsk.resize(maxTicks, 0.0);
    nonZeroCountBid.resize(maxTicks, 0);
    nonZeroCountAsk.resize(maxTicks, 0);
}

void OrderDepthProfileStats::clear() {
    numSnapshots = 0;
    maxTicks = 0;
    avgBid.clear();
    avgAsk.clear();
    stdBid.clear();
    stdAsk.clear();
    sumBid.clear();
    sumAsk.clear();
    sumSqBid.clear();
    sumSqAsk.clear();
    nonZeroCountBid.clear();
    nonZeroCountAsk.clear();
}

void OrderDepthProfileStats::compute() {
    if (numSnapshots == 0)
        Error::LIB_THROW("[OrderDepthProfileStats::compute] No snapshots accumulated to compute the depth profile.");
    avgBid.resize(maxTicks, 0.0);
    avgAsk.resize(maxTicks, 0.0);
    stdBid.resize(maxTicks, 0.0);
    stdAsk.resize(maxTicks, 0.0);
    for (size_t i = 0; i < maxTicks; ++i) {
        if (countMissingLevels ? (numSnapshots > 0) : (nonZeroCountBid[i] > 0)) {
            const size_t countBid = countMissingLevels ? numSnapshots : nonZeroCountBid[i];
            avgBid[i] = sumBid[i] / static_cast<double>(countBid);
            stdBid[i] = std::sqrt((sumSqBid[i] / static_cast<double>(countBid)) - (avgBid[i] * avgBid[i]));
        }
        if (countMissingLevels ? (numSnapshots > 0) : (nonZeroCountAsk[i] > 0)) {
            const size_t countAsk = countMissingLevels ? numSnapshots : nonZeroCountAsk[i];
            avgAsk[i] = sumAsk[i] / static_cast<double>(countAsk);
            stdAsk[i] = std::sqrt((sumSqAsk[i] / static_cast<double>(countAsk)) - (avgAsk[i] * avgAsk[i]));
        }
    }
}

std::string OrderDepthProfileStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"normalization\":\""       << normalization            << "\",\n"
        << "\"priceSpace\":\""          << priceSpace               << "\",\n"
        << "\"maxTicks\":"              << maxTicks                 << ",\n"
        << "\"minPriceTick\":"          << minPriceTick             << ",\n"
        << "\"countMissingLevels\":"    << countMissingLevels       << ",\n"
        << "\"numSnapshots\":"          << numSnapshots             << ",\n"
        << "\"avgBid\":"                << Utils::toString(avgBid)  << ",\n"
        << "\"avgAsk\":"                << Utils::toString(avgAsk)  << ",\n"
        << "\"stdBid\":"                << Utils::toString(stdBid)  << ",\n"
        << "\"stdAsk\":"                << Utils::toString(stdAsk)  << "\n"
        << "}";
    return oss.str();
}

void OrderFlowMemoryStats::set(const OrderFlowMemoryStatsConfig& config) {
    lags = config.lags;
}

void OrderFlowMemoryStats::accumulate(const int8_t tradeSign) {
    tradeSignsACF.add(tradeSign);
}

void OrderFlowMemoryStats::init() {
    numTrades = 0;
    meanTradeSign = 0.0;
    varTradeSign = 0.0;
    tradeSignsACF.clear();
    autocorrelations.clear();
    if (lags.empty())
        lags = std::vector<size_t>(DefaultLags.begin(), DefaultLags.end());
}

void OrderFlowMemoryStats::clear() {
    numTrades = 0;
    meanTradeSign = 0.0;
    varTradeSign = 0.0;
    tradeSignsACF.clear();
    autocorrelations.clear();
    lags.clear();
}

void OrderFlowMemoryStats::compute() {
    numTrades = tradeSignsACF.size();
    meanTradeSign = tradeSignsACF.getMean();
    varTradeSign = tradeSignsACF.getVariance();
    autocorrelations.resize(lags.size(), 0.0);
    for (size_t i = 0; i < lags.size(); ++i)
        autocorrelations[i] = tradeSignsACF.get(lags[i]);
}

std::string OrderFlowMemoryStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"numTrades\":"        << numTrades                         << ",\n"
        << "\"meanTradeSign\":"    << meanTradeSign                     << ",\n"
        << "\"varTradeSign\":"     << varTradeSign                      << ",\n"
        << "\"lags\":"             << Utils::toString(lags)             << ",\n"
        << "\"autocorrelations\":" << Utils::toString(autocorrelations) << "\n"
        << "}";
    return oss.str();
}

void PriceReturnScalingStats::set(const PriceReturnScalingStatsConfig& config) {
    priceType = config.priceType;
    logReturns = config.logReturns;
}

void PriceReturnScalingStats::init() {
    if (priceType == PriceType::NONE)
        Error::LIB_THROW("[PriceReturnScalingStats::init] Price type is NONE.");
    horizons.clear();
    sumReturns.clear();
    sumSqReturns.clear();
    counts.clear();
}

void PriceReturnScalingStats::clear() {
    horizons.clear();
    sumReturns.clear();
    sumSqReturns.clear();
    counts.clear();
}

void SpreadStats::set(const SpreadStatsConfig& config) {
    spreadHistogram.setBins(config.minSpread, config.maxSpread, config.numBins, config.binning);
}

void SpreadStats::accumulate(const double spread) {
    spreadHistogram.add(spread);
    spreadACF.add(spread);
}

void SpreadStats::init() {
    numSpreads = 0;
    meanSpread = 0.0;
    varSpread = 0.0;
    spreadHistogram.clear(); // clear existing data but retain bins
    spreadACF.clear();
    if (spreadHistogram.empty()) // check if bins are unset
        spreadHistogram.setBins(MinSpread, MaxSpread, NumBins, Statistics::Histogram::Binning::UNIFORM);
}

void SpreadStats::clear() {
    numSpreads = 0;
    meanSpread = 0.0;
    varSpread = 0.0;
    spreadHistogram.clear();
    spreadACF.clear();
}

void SpreadStats::compute() {
    numSpreads = spreadHistogram.getTotalCount();
    meanSpread = spreadHistogram.getMean();
    varSpread = spreadHistogram.getVariance();
}

std::string SpreadStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"numSpreads\":"     << numSpreads                   << ",\n"
        << "\"meanSpread\":"     << meanSpread                   << ",\n"
        << "\"varSpread\":"      << varSpread                    << ",\n"
        << "\"spreadHistogram\":" << spreadHistogram.getAsJson() << "\n"
        << "}";
    return oss.str();
}
}

#endif
