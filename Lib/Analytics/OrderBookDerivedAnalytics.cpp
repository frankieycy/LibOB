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

std::string toString(const EventTimeStats::PriceType& priceType) {
    switch (priceType) {
        case EventTimeStats::PriceType::MID:   return "Mid";
        case EventTimeStats::PriceType::MICRO: return "Micro";
        default:                               return "None";
    }
}

std::string toString(const OrderLifetimeStats::PriceSpaceDefinition& priceSpace) {
    switch (priceSpace) {
        case OrderLifetimeStats::PriceSpaceDefinition::DIFF_TO_MID:           return "DiffToMid";
        case OrderLifetimeStats::PriceSpaceDefinition::DIFF_TO_OWN_BEST:      return "DiffToOwnBest";
        case OrderLifetimeStats::PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST: return "DiffToOppositeBest";
        default:                                                              return "None";
    }
}

std::string toString(const OrderLifetimeStats::OrderDeathType& deathType) {
    switch (deathType) {
        case OrderLifetimeStats::OrderDeathType::CANCEL:  return "Cancel";
        case OrderLifetimeStats::OrderDeathType::EXECUTE: return "Execute";
        default:                                          return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::DepthNormalization& normalization) { return out << toString(normalization); }
std::ostream& operator<<(std::ostream& out, const OrderDepthProfileStats::PriceSpaceDefinition& priceSpace) { return out << toString(priceSpace); }
std::ostream& operator<<(std::ostream& out, const PriceReturnScalingStats::PriceType& priceType) { return out << toString(priceType); }
std::ostream& operator<<(std::ostream& out, const EventTimeStats::PriceType& priceType) { return out << toString(priceType); }
std::ostream& operator<<(std::ostream& out, const OrderLifetimeStats::PriceSpaceDefinition& priceSpace) { return out << toString(priceSpace); }
std::ostream& operator<<(std::ostream& out, const OrderLifetimeStats::OrderDeathType& deathType) { return out << toString(deathType); }

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
    horizons = config.horizons;
    priceType = config.priceType;
    logReturns = config.logReturns;
}

void PriceReturnScalingStats::accumulate(const OrderBookStatisticsByTimestamp& stats) {
    double price = Consts::NAN_DOUBLE;
    switch (priceType) {
        case PriceType::LAST_TRADE:
            price = stats.lastTradePrice;
            break;
        case PriceType::MID:
            price = stats.midPrice;
            break;
        case PriceType::MICRO:
            price = stats.microPrice;
            break;
        default:
            Error::LIB_THROW("[PriceReturnScalingStats::accumulate] Invalid price type.");
    }
    if (Consts::isNaN(price))
        return;
    prices.push_back(price);
    timestamps.push_back(stats.timestampTo);
}

void PriceReturnScalingStats::init() {
    if (priceType == PriceType::NONE)
        Error::LIB_THROW("[PriceReturnScalingStats::init] Price type is NONE.");
    prices.clear();
    timestamps.clear();
    sumReturns.clear();
    sumSqReturns.clear();
    counts.clear();
    varReturns.clear();
    if (horizons.empty())
        horizons = std::vector<uint64_t>(DefaultHorizons.begin(), DefaultHorizons.end());
}

void PriceReturnScalingStats::clear() {
    prices.clear();
    timestamps.clear();
    sumReturns.clear();
    sumSqReturns.clear();
    counts.clear();
    varReturns.clear();
    horizons.clear();
}

void PriceReturnScalingStats::compute() {
    const size_t numHorizons = horizons.size();
    sumReturns.resize(numHorizons, 0.0);
    sumSqReturns.resize(numHorizons, 0.0);
    counts.resize(numHorizons, 0);
    varReturns.resize(numHorizons, 0.0);
    const size_t numPrices = prices.size();
    for (size_t i = 0; i < numPrices; ++i) {
        const double price_i = prices[i];
        const uint64_t time_i = timestamps[i];
        for (size_t h = 0; h < numHorizons; ++h) {
            const uint64_t horizon = horizons[h];
            size_t j = i + 1;
            while (j < numPrices && (timestamps[j] - time_i) < horizon)
                ++j; // such that timestamps[j] - time_i >= horizon
            if (j < numPrices) {
                const double price_j = prices[j];
                double ret = price_j - price_i;
                if (logReturns) {
                    if (price_i <= 0.0 || price_j <= 0.0)
                        continue; // skip invalid prices for log returns
                    ret = std::log(price_j / price_i);
                }
                sumReturns[h] += ret;
                sumSqReturns[h] += ret * ret;
                ++counts[h];
            }
        }
    }
    for (size_t h = 0; h < numHorizons; ++h) {
        if (counts[h] > 0) {
            const double meanRet = sumReturns[h] / static_cast<double>(counts[h]);
            varReturns[h] = (sumSqReturns[h] / static_cast<double>(counts[h])) - (meanRet * meanRet);
        }
    }
}

std::string PriceReturnScalingStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"priceType\":\""   << priceType                    << "\",\n"
        << "\"logReturns\":"    << logReturns                   << ",\n"
        << "\"horizons\":"      << Utils::toString(horizons)    << ",\n"
        << "\"varReturns\":"    << Utils::toString(varReturns)  << "\n"
        << "}";
    return oss.str();
}

void SpreadStats::set(const SpreadStatsConfig& config) {
    spreadHistogram.setBins(config.minSpread, config.maxSpread, config.numBins, config.binning);
    lags = config.lags;
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
    autocorrelations.clear();
    if (spreadHistogram.empty()) // check if bins are unset
        spreadHistogram.setBins(MinSpread, MaxSpread, NumBins, Statistics::Histogram::Binning::UNIFORM);
    if (lags.empty())
        lags = std::vector<size_t>(DefaultLags.begin(), DefaultLags.end());
}

void SpreadStats::clear() {
    numSpreads = 0;
    meanSpread = 0.0;
    varSpread = 0.0;
    spreadHistogram.clear();
    spreadACF.clear();
    autocorrelations.clear();
    lags.clear();
}

void SpreadStats::compute() {
    numSpreads = spreadHistogram.getTotalCount();
    meanSpread = spreadHistogram.getMean();
    varSpread = spreadHistogram.getVariance();
    autocorrelations.resize(lags.size(), 0.0);
    for (size_t i = 0; i < lags.size(); ++i)
        autocorrelations[i] = spreadACF.get(lags[i]);
}

std::string SpreadStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"numSpreads\":"       << numSpreads                   << ",\n"
        << "\"meanSpread\":"       << meanSpread                   << ",\n"
        << "\"varSpread\":"        << varSpread                    << ",\n"
        << "\"spreadHistogram\":"  << spreadHistogram.getAsJson()  << ",\n"
        << "\"lags\":"             << Utils::toString(lags)        << ",\n"
        << "\"autocorrelations\":" << Utils::toString(autocorrelations) << "\n"
        << "}";
    return oss.str();
}

void EventTimeStats::set(const EventTimeStatsConfig& config) {
    eventsBetweenPriceMoves.setBins(config.minEventTime, config.maxEventTime, config.numBins, config.binning);
    priceType = config.priceType;
}

void EventTimeStats::accumulate(const OrderBookStatisticsByTimestamp& stats) {
    double price = Consts::NAN_DOUBLE;
    switch (priceType) {
        case PriceType::MID:
            price = stats.midPrice;
            break;
        case PriceType::MICRO:
            price = stats.microPrice;
            break;
        default:
            Error::LIB_THROW("[EventTimeStats::accumulate] Invalid price type.");
    }
    if (Consts::isNaN(price))
        return;
    const size_t numEvents = stats.cumNumNewLimitOrders // all order events
                           + stats.cumNumNewMarketOrders
                           + stats.cumNumCancelOrders
                           + stats.cumNumModifyPriceOrders
                           + stats.cumNumModifyQuantityOrders;
    prices.push_back(price);
    eventCounts.push_back(numEvents);
    timestamps.push_back(stats.timestampTo);
}

void EventTimeStats::init() {
    if (priceType == PriceType::NONE)
        Error::LIB_THROW("[EventTimeStats::init] Price type is NONE.");
    numPriceTicks = 0;
    meanPriceTicks = 0.0;
    varPriceTicks = 0.0;
    prices.clear();
    eventCounts.clear();
    timestamps.clear();
    eventsBetweenPriceMoves.clear(); // clear existing data but retain bins
    if (eventsBetweenPriceMoves.empty()) // check if bins are unset
        eventsBetweenPriceMoves.setBins(MinEventTime, MaxEventTime, NumBins, Statistics::Histogram::Binning::UNIFORM);
}

void EventTimeStats::clear() {
    numPriceTicks = 0;
    meanPriceTicks = 0.0;
    varPriceTicks = 0.0;
    prices.clear();
    eventCounts.clear();
    timestamps.clear();
    eventsBetweenPriceMoves.clear();
}

void EventTimeStats::compute() {
    const size_t numPrices = prices.size();
    if (numPrices == 0)
        Error::LIB_THROW("[EventTimeStats::compute] No price data accumulated to compute event time stats.");
    std::vector<size_t> eventTimes;
    size_t cumEventTime = eventCounts[0];
    for (size_t i = 1; i < numPrices; ++i) {
        if (prices[i] != prices[i - 1]) { // price tick detected
            eventTimes.push_back(cumEventTime);
            cumEventTime = eventCounts[i];
        } else {
            cumEventTime += eventCounts[i];
        }
    }
    numPriceTicks = eventTimes.size();
    if (numPriceTicks == 0)
        Error::LIB_THROW("[EventTimeStats::compute] No price ticks detected to compute event time stats.");
    double sumEvents = 0.0;
    double sumSqEvents = 0.0;
    for (const auto& et : eventTimes) {
        sumEvents += static_cast<double>(et);
        sumSqEvents += static_cast<double>(et * et);
        eventsBetweenPriceMoves.add(static_cast<double>(et));
    }
    meanPriceTicks = sumEvents / static_cast<double>(numPriceTicks);
    varPriceTicks = (sumSqEvents / static_cast<double>(numPriceTicks)) - (meanPriceTicks * meanPriceTicks);
}

std::string EventTimeStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"priceType\":\""       << priceType                        << "\",\n"
        << "\"numPriceTicks\":"     << numPriceTicks                    << ",\n"
        << "\"meanPriceTicks\":"    << meanPriceTicks                   << ",\n"
        << "\"varPriceTicks\":"     << varPriceTicks                    << ",\n"
        << "\"eventsBetweenPriceMoves\":" << eventsBetweenPriceMoves.getAsJson() << "\n"
        << "}";
    return oss.str();
}

void OrderLifetimeStats::set(const OrderLifetimeStatsConfig& config) {
    priceSpace = config.priceSpace;
    maxTicks = config.maxTicks;
    minPriceTick = config.minPriceTick;
    minLifetime = config.minLifetime;
    maxLifetime = config.maxLifetime;
    numBins = config.numBins;
}

void OrderLifetimeStats::accumulate(const OrderBookStatisticsByTimestamp& stats, const Market::Side side) {
    // ticks the current timestamp and reference price, but does no report-specific processing yet (e.g. order placements)
    currentTimestamp = stats.timestampTo;
    switch (priceSpace) {
        case PriceSpaceDefinition::DIFF_TO_MID:
            currentRefPrice = stats.midPrice;
            break;
        case PriceSpaceDefinition::DIFF_TO_OWN_BEST:
            currentRefPrice = (side == Market::Side::BUY) ? stats.bestBidPrice : stats.bestAskPrice;
            break;
        case PriceSpaceDefinition::DIFF_TO_OPPOSITE_BEST:
            currentRefPrice = (side == Market::Side::SELL) ? stats.bestAskPrice : stats.bestBidPrice;
            break;
        default:
            Error::LIB_THROW("[OrderLifetimeStats::accumulate] Invalid price space definition.");
    }
}

void OrderLifetimeStats::onOrderPlacement(const uint64_t orderId, const Market::Side side, const uint32_t quantity, const double price) {
    const long long priceDiffTicks = Maths::countPriceTicks(std::abs(price - currentRefPrice), minPriceTick);
    orderBirths.try_emplace(orderId, currentTimestamp, side, quantity, price, currentRefPrice, priceDiffTicks);
}

void OrderLifetimeStats::onOrderCancel(const uint64_t orderId) {
    const auto it = orderBirths.find(orderId);
    if (it == orderBirths.end())
        return; // order not found
    const OrderBirth& birth = it->second;
    const double lifetime = static_cast<double>(currentTimestamp - birth.timestamp);
    const size_t priceDiffTicksIndex = static_cast<size_t>(birth.priceDiffTicks);
    if (priceDiffTicksIndex < maxTicks) {
        if (birth.side == Market::Side::BUY)
            lifetimeToCancelByBidPriceBucket[priceDiffTicksIndex].add(lifetime);
        else
            lifetimeToCancelByAskPriceBucket[priceDiffTicksIndex].add(lifetime);
    }
    orderBirths.erase(it);
}

void OrderLifetimeStats::onOrderPartialCancel(const uint64_t orderId, const uint32_t cancelQuantity) {
    const auto it = orderBirths.find(orderId);
    if (it == orderBirths.end())
        return;
    OrderBirth& birth = it->second;
    birth.quantityAlive -= cancelQuantity;
    if (birth.quantityAlive > 0)
        return; // order still alive
    const double lifetime = static_cast<double>(currentTimestamp - birth.timestamp);
    const size_t priceDiffTicksIndex = static_cast<size_t>(birth.priceDiffTicks);
    if (priceDiffTicksIndex < maxTicks) {
        if (birth.side == Market::Side::BUY)
            lifetimeToCancelByBidPriceBucket[priceDiffTicksIndex].add(lifetime);
        else
            lifetimeToCancelByAskPriceBucket[priceDiffTicksIndex].add(lifetime);
    }
    orderBirths.erase(it);
}

void OrderLifetimeStats::onOrderExecute(const uint64_t orderId, const uint32_t executedQuantity) {
    const auto it = orderBirths.find(orderId);
    if (it == orderBirths.end())
        return;
    OrderBirth& birth = it->second;
    birth.quantityAlive -= executedQuantity;
    if (birth.quantityAlive > 0)
        return; // order still alive
    const double lifetime = static_cast<double>(currentTimestamp - birth.timestamp);
    const size_t priceDiffTicksIndex = static_cast<size_t>(birth.priceDiffTicks);
    if (priceDiffTicksIndex < maxTicks) {
        if (birth.side == Market::Side::BUY)
            lifetimeToExecuteByBidPriceBucket[priceDiffTicksIndex].add(lifetime);
        else
            lifetimeToExecuteByAskPriceBucket[priceDiffTicksIndex].add(lifetime);
    }
}

void OrderLifetimeStats::init() {
    if (priceSpace == PriceSpaceDefinition::NONE)
        Error::LIB_THROW("[OrderLifetimeStats::init] Price space definition is NONE.");
    orderBirths.clear();
    meanLifetimeToCancelByBidPriceBucket.resize(maxTicks, 0.0);
    meanLifetimeToCancelByAskPriceBucket.resize(maxTicks, 0.0);
    meanLifetimeToExecuteByBidPriceBucket.resize(maxTicks, 0.0);
    meanLifetimeToExecuteByAskPriceBucket.resize(maxTicks, 0.0);
    lifetimeToCancelByBidPriceBucket.resize(maxTicks);
    lifetimeToCancelByAskPriceBucket.resize(maxTicks);
    lifetimeToExecuteByBidPriceBucket.resize(maxTicks);
    lifetimeToExecuteByAskPriceBucket.resize(maxTicks);
    for (size_t i = 0; i < maxTicks; ++i) {
        lifetimeToCancelByBidPriceBucket[i].setBins(minLifetime.value_or(MinLifetime), maxLifetime.value_or(MaxLifetime), numBins.value_or(NumBins), Statistics::Histogram::Binning::UNIFORM);
        lifetimeToCancelByAskPriceBucket[i].setBins(minLifetime.value_or(MinLifetime), maxLifetime.value_or(MaxLifetime), numBins.value_or(NumBins), Statistics::Histogram::Binning::UNIFORM);
        lifetimeToExecuteByBidPriceBucket[i].setBins(minLifetime.value_or(MinLifetime), maxLifetime.value_or(MaxLifetime), numBins.value_or(NumBins), Statistics::Histogram::Binning::UNIFORM);
        lifetimeToExecuteByAskPriceBucket[i].setBins(minLifetime.value_or(MinLifetime), maxLifetime.value_or(MaxLifetime), numBins.value_or(NumBins), Statistics::Histogram::Binning::UNIFORM);
    }
}

void OrderLifetimeStats::clear() {
    maxTicks = 0;
    orderBirths.clear();
    meanLifetimeToCancelByBidPriceBucket.clear();
    meanLifetimeToCancelByAskPriceBucket.clear();
    meanLifetimeToExecuteByBidPriceBucket.clear();
    meanLifetimeToExecuteByAskPriceBucket.clear();
    lifetimeToCancelByBidPriceBucket.clear();
    lifetimeToCancelByAskPriceBucket.clear();
    lifetimeToExecuteByBidPriceBucket.clear();
    lifetimeToExecuteByAskPriceBucket.clear();
}

void OrderLifetimeStats::compute() {
    for (size_t i = 0; i < maxTicks; ++i) {
        meanLifetimeToCancelByBidPriceBucket[i] = lifetimeToCancelByBidPriceBucket[i].getMean();
        meanLifetimeToCancelByAskPriceBucket[i] = lifetimeToCancelByAskPriceBucket[i].getMean();
        meanLifetimeToExecuteByBidPriceBucket[i] = lifetimeToExecuteByBidPriceBucket[i].getMean();
        meanLifetimeToExecuteByAskPriceBucket[i] = lifetimeToExecuteByAskPriceBucket[i].getMean();
    }
}

std::string OrderLifetimeStats::getAsJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "\"priceSpace\":\""    << priceSpace   << "\",\n"
        << "\"maxTicks\":"        << maxTicks     << ",\n"
        << "\"minPriceTick\":"    << minPriceTick << ",\n"
        << "\"meanLifetimeToCancelByBidPriceBucket\":"    << Utils::toString(meanLifetimeToCancelByBidPriceBucket)    << ",\n"
        << "\"meanLifetimeToCancelByAskPriceBucket\":"    << Utils::toString(meanLifetimeToCancelByAskPriceBucket)    << ",\n"
        << "\"meanLifetimeToExecuteByBidPriceBucket\":"   << Utils::toString(meanLifetimeToExecuteByBidPriceBucket)   << ",\n"
        << "\"meanLifetimeToExecuteByAskPriceBucket\":"   << Utils::toString(meanLifetimeToExecuteByAskPriceBucket)   << "\n"
        << "}";
    return oss.str();
}
}

#endif
