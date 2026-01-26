#ifndef ORDER_BOOK_OBSERVABLES_CPP
#define ORDER_BOOK_OBSERVABLES_CPP
#include "Utils/Utils.hpp"
#include "Market/Trade.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/OrderBookObservables.hpp"

namespace Analytics {
using namespace Utils;
using Utils::operator<<;

std::ostream& operator<<(std::ostream& out, const OrderBookTopLevelsSnapshot& snapshot) { return out << snapshot.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const OrderBookAggregateStatistics& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const OrderBookStatisticsByTimestamp& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const OrderEventProcessingLatency& latency) { return out << latency.getAsJson(); }

void OrderBookTopLevelsSnapshot::constructFrom(const std::shared_ptr<const Exchange::IMatchingEngine>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderBookTopLevelsSnapshot] Matching engine is null.");
    lastTrade = matchingEngine->getLastTrade();
    bidBookTopPrices = isFullBook ? matchingEngine->getBidBookPriceVector() : matchingEngine->getBidBookPriceVector(numLevels);
    askBookTopPrices = isFullBook ? matchingEngine->getAskBookPriceVector() : matchingEngine->getAskBookPriceVector(numLevels);
    bidBookTopSizes = isFullBook ? matchingEngine->getBidBookSizeVector() : matchingEngine->getBidBookSizeVector(numLevels);
    askBookTopSizes = isFullBook ? matchingEngine->getAskBookSizeVector() : matchingEngine->getAskBookSizeVector(numLevels);
}

void OrderBookTopLevelsSnapshot::clear() {
    numLevels = 0;
    isFullBook = false;
    lastTrade = nullptr;
    bidBookTopPrices.clear();
    askBookTopPrices.clear();
    bidBookTopSizes.clear();
    askBookTopSizes.clear();
}

std::string OrderBookTopLevelsSnapshot::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"NumLevels\":"  << numLevels << ","
    "\"IsFullBook\":" << isFullBook << ","
    "\"LastTrade\":"  << (lastTrade ? lastTrade->getAsJson() : "null") << ",";
    oss << "\"BidBookTopLevels\":[";
    for (size_t i = 0; i < bidBookTopPrices.size(); ++i) {
        oss << "(" << bidBookTopPrices[i] << "," << bidBookTopSizes[i] << ")";
        if (i < bidBookTopPrices.size() - 1)
            oss << ",";
    }
    oss << "],\"AskBookTopLevels\":[";
    for (size_t i = 0; i < askBookTopPrices.size(); ++i) {
        oss << "(" << askBookTopPrices[i] << "," << askBookTopSizes[i] << ")";
        if (i < askBookTopPrices.size() - 1)
            oss << ",";
    }
    oss << "]}";
    return oss.str();
}

std::string OrderBookTopLevelsSnapshot::getAsCsv() const {
    // format: BidNumLevels,AskNumLevels,BidPrice1,BidSize1,...,AskPrice1,AskSize1,...
    // note that the last trade information is omitted
    std::ostringstream oss;
    oss << bidBookTopPrices.size() << "," << askBookTopPrices.size() << ",";
    for (size_t i = 0; i < bidBookTopPrices.size(); ++i) {
        oss << bidBookTopPrices[i] << "," << bidBookTopSizes[i];
        if (i < bidBookTopPrices.size() - 1)
            oss << ",";
    }
    oss << ",";
    for (size_t i = 0; i < askBookTopPrices.size(); ++i) {
        oss << askBookTopPrices[i] << "," << askBookTopSizes[i];
        if (i < askBookTopPrices.size() - 1)
            oss << ",";
    }
    return oss.str();
}

std::string OrderBookTopLevelsSnapshot::getAsTable() const {
    std::ostringstream oss;
    oss <<
    "---------------------------------------------------------\n"
    "|              Order Book Top Levels Snapshot           |\n"
    "---------------------------------------------------------\n"
    "| NumLevels                  |" << std::setw(25) << numLevels   << " |\n"
    "| IsFullBook                 |" << std::setw(25) << isFullBook  << " |\n";
    if (lastTrade) {
        oss <<
        "---------------------------------------------------------\n"
        "|                    Last Trade Details                 |\n"
        "---------------------------------------------------------\n"
        "| Id                         |" << std::setw(25) << lastTrade->getId()               << " |\n"
        "| Timestamp                  |" << std::setw(25) << lastTrade->getTimestamp()        << " |\n"
        "| BuyId                      |" << std::setw(25) << lastTrade->getBuyId()            << " |\n"
        "| SellId                     |" << std::setw(25) << lastTrade->getSellId()           << " |\n"
        "| Quantity                   |" << std::setw(25) << lastTrade->getQuantity()         << " |\n"
        "| Price                      |" << std::setw(25) << lastTrade->getPrice()            << " |\n"
        "| IsBuyLimitOrder            |" << std::setw(25) << lastTrade->getIsBuyLimitOrder()  << " |\n"
        "| IsSellLimitOrder           |" << std::setw(25) << lastTrade->getIsSellLimitOrder() << " |\n"
        "| IsBuyInitiated             |" << std::setw(25) << lastTrade->getIsBuyInitiated()     << " |\n";
    }
    oss <<
    "---------------------------------------------------------\n"
    "|                      Order Book Size                  |\n"
    "---------------------------------------------------------\n"
    "| BID Size | BID Price || Level || ASK Price | ASK Size |\n"
    "---------------------------------------------------------\n";
    uint level = 1;
    for (size_t i = 0; i < std::max(bidBookTopPrices.size(), askBookTopPrices.size()); ++i) {
        oss << "|";
        if (i < bidBookTopPrices.size()) {
            oss << std::setw(9) << bidBookTopSizes[i] << " | "
                << std::fixed << std::setprecision(2)
                << std::setw(9) << bidBookTopPrices[i] << " || "
                << std::setw(5) << level << " || ";
        } else {
            oss << "          |           || ";
            oss << std::setw(5) << level << " || ";
        }
        if (i < askBookTopPrices.size()) {
            oss << std::fixed << std::setprecision(2)
                << std::setw(9) << askBookTopPrices[i] << " | "
                << std::setw(8) << askBookTopSizes[i] << " |\n";
        } else {
            oss << "          |          |\n";
        }
        if (++level > numLevels && !isFullBook)
            break;
    }
    oss << "---------------------------------------------------------\n";
    return oss.str();
}

std::string OrderBookAggregateStatistics::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"TimestampFrom\":"              << timestampFrom              << ","
    "\"TimestampTo\":"                << timestampTo                << ","
    "\"AggNumNewLimitOrders\":"       << aggNumNewLimitOrders       << ","
    "\"AggNumNewMarketOrders\":"      << aggNumNewMarketOrders      << ","
    "\"AggNumCancelOrders\":"         << aggNumCancelOrders         << ","
    "\"AggNumModifyPriceOrders\":"    << aggNumModifyPriceOrders    << ","
    "\"AggNumModifyQuantityOrders\":" << aggNumModifyQuantityOrders << ","
    "\"AggNumTrades\":"               << aggNumTrades               << ","
    "\"AggTradeVolume\":"             << aggTradeVolume             << ","
    "\"AggTradeNotional\":"           << aggTradeNotional;
    oss << "}";
    return oss.str();
}

std::string OrderBookAggregateStatistics::getAsCsv() const {
    std::ostringstream oss;
    oss <<
    timestampFrom              << "," <<
    timestampTo                << "," <<
    aggNumNewLimitOrders       << "," <<
    aggNumNewMarketOrders      << "," <<
    aggNumCancelOrders         << "," <<
    aggNumModifyPriceOrders    << "," <<
    aggNumModifyQuantityOrders << "," <<
    aggNumTrades               << "," <<
    aggTradeVolume             << "," <<
    aggTradeNotional;
    return oss.str();
}

std::string OrderBookAggregateStatistics::getAsTable() const {
    std::ostringstream oss;
    oss <<
    "---------------------------------------------------------\n"
    "|             Order Book Aggregate Statistics           |\n"
    "---------------------------------------------------------\n"
    "| TimestampFrom              |" << std::setw(25) << timestampFrom              << " |\n"
    "| TimestampTo                |" << std::setw(25) << timestampTo                << " |\n"
    "| AggNumNewLimitOrders       |" << std::setw(25) << aggNumNewLimitOrders       << " |\n"
    "| AggNumNewMarketOrders      |" << std::setw(25) << aggNumNewMarketOrders      << " |\n"
    "| AggNumCancelOrders         |" << std::setw(25) << aggNumCancelOrders         << " |\n"
    "| AggNumModifyPriceOrders    |" << std::setw(25) << aggNumModifyPriceOrders    << " |\n"
    "| AggNumModifyQuantityOrders |" << std::setw(25) << aggNumModifyQuantityOrders << " |\n"
    "| AggNumTrades               |" << std::setw(25) << aggNumTrades               << " |\n"
    "| AggTradeVolume             |" << std::setw(25) << aggTradeVolume             << " |\n"
    "| AggTradeNotional           |" << std::setw(25) << aggTradeNotional           << " |\n"
    "---------------------------------------------------------\n";
    return oss.str();
}

void OrderBookStatisticsByTimestamp::constructFrom(
    const std::shared_ptr<const Exchange::IMatchingEngine>& matchingEngine,
    const OrderBookAggregateStatistics& orderBookAggregateStatistics,
    const OrderBookAggregateStatistics& orderBookAggregateStatisticsCache) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderBookStatisticsByTimestamp] Matching engine is null.");
    timestampFrom = orderBookAggregateStatisticsCache.timestampTo; // from-time exclusive
    timestampTo = orderBookAggregateStatistics.timestampTo; // to-time inclusive
    cumNumNewLimitOrders = orderBookAggregateStatistics.aggNumNewLimitOrders - orderBookAggregateStatisticsCache.aggNumNewLimitOrders;
    cumNumNewMarketOrders = orderBookAggregateStatistics.aggNumNewMarketOrders - orderBookAggregateStatisticsCache.aggNumNewMarketOrders;
    cumNumCancelOrders = orderBookAggregateStatistics.aggNumCancelOrders - orderBookAggregateStatisticsCache.aggNumCancelOrders;
    cumNumModifyPriceOrders = orderBookAggregateStatistics.aggNumModifyPriceOrders - orderBookAggregateStatisticsCache.aggNumModifyPriceOrders;
    cumNumModifyQuantityOrders = orderBookAggregateStatistics.aggNumModifyQuantityOrders - orderBookAggregateStatisticsCache.aggNumModifyQuantityOrders;
    cumNumTrades = orderBookAggregateStatistics.aggNumTrades - orderBookAggregateStatisticsCache.aggNumTrades;
    cumTradeVolume = orderBookAggregateStatistics.aggTradeVolume - orderBookAggregateStatisticsCache.aggTradeVolume;
    cumTradeNotional = orderBookAggregateStatistics.aggTradeNotional - orderBookAggregateStatisticsCache.aggTradeNotional;
    bestBidPrice = matchingEngine->getBestBidPrice();
    bestAskPrice = matchingEngine->getBestAskPrice();
    midPrice = matchingEngine->getMidPrice();
    microPrice = matchingEngine->getMicroPrice();
    spread = matchingEngine->getSpread();
    halfSpread = matchingEngine->getHalfSpread();
    orderImbalance = matchingEngine->getOrderImbalance();
    bestBidSize = matchingEngine->getBestBidSize();
    bestAskSize = matchingEngine->getBestAskSize();
    lastTradePrice = matchingEngine->getLastTradePrice();
    lastTradeQuantity = matchingEngine->getLastTradeSize();
    lastTradeIsBuyInitiated = matchingEngine->getLastTradeIsBuyInitiated();
    topLevelsSnapshot.constructFrom(matchingEngine);
}

void OrderBookStatisticsByTimestamp::clear() {
    timestampFrom = 0;
    timestampTo = 0;
    cumNumNewLimitOrders = 0;
    cumNumNewMarketOrders = 0;
    cumNumCancelOrders = 0;
    cumNumModifyPriceOrders = 0;
    cumNumModifyQuantityOrders = 0;
    cumNumTrades = 0;
    cumTradeVolume = 0;
    cumTradeNotional = 0.0;
    bestBidPrice = Utils::Consts::NAN_DOUBLE;
    bestAskPrice = Utils::Consts::NAN_DOUBLE;
    midPrice = Utils::Consts::NAN_DOUBLE;
    microPrice = Utils::Consts::NAN_DOUBLE;
    spread = Utils::Consts::NAN_DOUBLE;
    halfSpread = Utils::Consts::NAN_DOUBLE;
    orderImbalance = Utils::Consts::NAN_DOUBLE;
    bestBidSize = 0;
    bestAskSize = 0;
    lastTradePrice = Utils::Consts::NAN_DOUBLE;
    lastTradeQuantity = 0;
    topLevelsSnapshot.clear();
}

std::string OrderBookStatisticsByTimestamp::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"TimestampFrom\":"              << timestampFrom              << ","
    "\"TimestampTo\":"                << timestampTo                << ","
    "\"CumNumNewLimitOrders\":"       << cumNumNewLimitOrders       << ","
    "\"CumNumNewMarketOrders\":"      << cumNumNewMarketOrders      << ","
    "\"CumNumCancelOrders\":"         << cumNumCancelOrders         << ","
    "\"CumNumModifyPriceOrders\":"    << cumNumModifyPriceOrders    << ","
    "\"CumNumModifyQuantityOrders\":" << cumNumModifyQuantityOrders << ","
    "\"CumNumTrades\":"               << cumNumTrades               << ","
    "\"CumTradeVolume\":"             << cumTradeVolume             << ","
    "\"CumTradeNotional\":"           << cumTradeNotional           << ","
    "\"BestBidPrice\":"               << bestBidPrice               << ","
    "\"BestAskPrice\":"               << bestAskPrice               << ","
    "\"MidPrice\":"                   << midPrice                   << ","
    "\"MicroPrice\":"                 << microPrice                 << ","
    "\"Spread\":"                     << spread                     << ","
    "\"HalfSpread\":"                 << halfSpread                 << ","
    "\"OrderImbalance\":"             << orderImbalance             << ","
    "\"BestBidSize\":"                << bestBidSize                << ","
    "\"BestAskSize\":"                << bestAskSize                << ","
    "\"LastTradePrice\":"             << lastTradePrice             << ","
    "\"LastTradeQuantity\":"          << lastTradeQuantity          << ","
    "\"TopLevelsSnapshot\":"          << topLevelsSnapshot;
    oss << "}";
    return oss.str();
}

std::string OrderBookStatisticsByTimestamp::getAsCsv() const {
    std::ostringstream oss;
    oss <<
    timestampFrom              << "," <<
    timestampTo                << "," <<
    cumNumNewLimitOrders       << "," <<
    cumNumNewMarketOrders      << "," <<
    cumNumCancelOrders         << "," <<
    cumNumModifyPriceOrders    << "," <<
    cumNumModifyQuantityOrders << "," <<
    cumNumTrades               << "," <<
    cumTradeVolume             << "," <<
    cumTradeNotional           << "," <<
    bestBidPrice               << "," <<
    bestAskPrice               << "," <<
    midPrice                   << "," <<
    microPrice                 << "," <<
    spread                     << "," <<
    halfSpread                 << "," <<
    orderImbalance             << "," <<
    bestBidSize                << "," <<
    bestAskSize                << "," <<
    lastTradePrice             << "," <<
    lastTradeQuantity          << "," <<
    topLevelsSnapshot.getAsCsv();
    return oss.str();
}

std::string OrderBookStatisticsByTimestamp::getAsTable() const {
    std::ostringstream oss;
    oss <<
    "---------------------------------------------------------\n"
    "|           Order Book Statistics By Timestamp          |\n"
    "---------------------------------------------------------\n"
    "| TimestampFrom              |" << std::setw(25) << timestampFrom              << " |\n"
    "| TimestampTo                |" << std::setw(25) << timestampTo                << " |\n"
    "| CumNumNewLimitOrders       |" << std::setw(25) << cumNumNewLimitOrders       << " |\n"
    "| CumNumNewMarketOrders      |" << std::setw(25) << cumNumNewMarketOrders      << " |\n"
    "| CumNumCancelOrders         |" << std::setw(25) << cumNumCancelOrders         << " |\n"
    "| CumNumModifyPriceOrders    |" << std::setw(25) << cumNumModifyPriceOrders    << " |\n"
    "| CumNumModifyQuantityOrders |" << std::setw(25) << cumNumModifyQuantityOrders << " |\n"
    "| CumNumTrades               |" << std::setw(25) << cumNumTrades               << " |\n"
    "| CumTradeVolume             |" << std::setw(25) << cumTradeVolume             << " |\n"
    "| CumTradeNotional           |" << std::setw(25) << cumTradeNotional           << " |\n"
    "| BestBidPrice               |" << std::setw(25) << bestBidPrice               << " |\n"
    "| BestAskPrice               |" << std::setw(25) << bestAskPrice               << " |\n"
    "| MidPrice                   |" << std::setw(25) << midPrice                   << " |\n"
    "| MicroPrice                 |" << std::setw(25) << microPrice                 << " |\n"
    "| Spread                     |" << std::setw(25) << spread                     << " |\n"
    "| HalfSpread                 |" << std::setw(25) << halfSpread                 << " |\n"
    "| OrderImbalance             |" << std::setw(25) << orderImbalance             << " |\n"
    "| BestBidSize                |" << std::setw(25) << bestBidSize                << " |\n"
    "| BestAskSize                |" << std::setw(25) << bestAskSize                << " |\n"
    "| LastTradePrice             |" << std::setw(25) << lastTradePrice             << " |\n"
    "| LastTradeQuantity          |" << std::setw(25) << lastTradeQuantity          << " |\n"
    "---------------------------------------------------------\n";
    oss << topLevelsSnapshot.getAsTable();
    return oss.str();
}

std::string OrderEventProcessingLatency::getAsJson() const {
    std::ostringstream oss;
    oss << "{" <<
    "\"Timestamp\":"        << timestamp        << "," <<
    "\"EventId\":"          << eventId          << "," <<
    "\"LatencyNs\":"        << latency          << "," <<
    "\"EventType\":"        << eventType        << "," <<
    "\"Event\":"            << (event ? event->getAsJson() : "null");
    oss << "}";
    return oss.str();
}

std::string OrderEventProcessingLatency::getAsCsv() const {
    std::ostringstream oss;
    oss <<
    timestamp << "," <<
    eventId   << "," <<
    latency   << "," <<
    eventType;
    return oss.str();
}

std::string OrderEventProcessingLatency::getAsTable() const {
    std::ostringstream oss;
    oss <<
    "---------------------------------------------------------\n"
    "|           Order Event Processing Latency              |\n"
    "---------------------------------------------------------\n"
    "| Timestamp                  |" << std::setw(25) << timestamp  << " |\n"
    "| EventId                    |" << std::setw(25) << eventId    << " |\n"
    "| LatencyNs                  |" << std::setw(25) << latency    << " |\n"
    "| EventType                  |" << std::setw(25) << eventType  << " |\n"
    "---------------------------------------------------------\n";
    return oss.str();
}
}

#endif
