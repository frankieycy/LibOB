#ifndef MATCHING_ENGINE_MONITOR_CPP
#define MATCHING_ENGINE_MONITOR_CPP
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Parser/LobsterDataParser.hpp"

namespace Analytics {
using namespace Utils;

std::string to_string(const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) {
    switch (strategy) {
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK:  return "TopOfBookTick";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_ORDER_EVENT:  return "EachOrderEvent";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_MARKET_ORDER: return "EachMarketOrder";
        case MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy::EACH_TRADE:        return "EachTrade";
        default:                                                                             return "Null";
    }
}

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsTimestampStrategy& strategy) { return out << to_string(strategy); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookTopLevelsSnapshot& snapshot) { return out << snapshot.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookAggregateStatistics& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderBookStatisticsByTimestamp& stats) { return out << stats.getAsJson(); }

std::ostream& operator<<(std::ostream& out, const MatchingEngineMonitor::OrderEventProcessingLatency& latency) { return out << latency.getAsJson(); }

void MatchingEngineMonitor::OrderBookTopLevelsSnapshot::constructFrom(const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine) {
    if (!matchingEngine)
        Error::LIB_THROW("[OrderBookTopLevelsSnapshot] Matching engine is null.");
    lastTrade = matchingEngine->getLastTrade();
    bidBookTopPrices = isFullBook ? matchingEngine->getBidBookPriceVector() : matchingEngine->getBidBookPriceVector(numLevels);
    askBookTopPrices = isFullBook ? matchingEngine->getAskBookPriceVector() : matchingEngine->getAskBookPriceVector(numLevels);
    bidBookTopSizes = isFullBook ? matchingEngine->getBidBookSizeVector() : matchingEngine->getBidBookSizeVector(numLevels);
    askBookTopSizes = isFullBook ? matchingEngine->getAskBookSizeVector() : matchingEngine->getAskBookSizeVector(numLevels);
}

void MatchingEngineMonitor::OrderBookTopLevelsSnapshot::clear() {
    numLevels = 0;
    isFullBook = false;
    lastTrade = nullptr;
    bidBookTopPrices.clear();
    askBookTopPrices.clear();
    bidBookTopSizes.clear();
    askBookTopSizes.clear();
}

std::string MatchingEngineMonitor::OrderBookTopLevelsSnapshot::getAsJson() const {
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

std::string MatchingEngineMonitor::OrderBookTopLevelsSnapshot::getAsCsv() const {
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

std::string MatchingEngineMonitor::OrderBookTopLevelsSnapshot::getAsTable() const {
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

std::string MatchingEngineMonitor::OrderBookAggregateStatistics::getAsJson() const {
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

std::string MatchingEngineMonitor::OrderBookAggregateStatistics::getAsCsv() const {
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

std::string MatchingEngineMonitor::OrderBookAggregateStatistics::getAsTable() const {
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

void MatchingEngineMonitor::OrderBookStatisticsByTimestamp::constructFrom(
    const std::shared_ptr<const Exchange::MatchingEngineBase>& matchingEngine,
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
    topLevelsSnapshot.constructFrom(matchingEngine);
}

void MatchingEngineMonitor::OrderBookStatisticsByTimestamp::clear() {
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

std::string MatchingEngineMonitor::OrderBookStatisticsByTimestamp::getAsJson() const {
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

std::string MatchingEngineMonitor::OrderBookStatisticsByTimestamp::getAsCsv() const {
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

std::string MatchingEngineMonitor::OrderBookStatisticsByTimestamp::getAsTable() const {
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

std::string MatchingEngineMonitor::OrderEventProcessingLatency::getAsJson() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderEventProcessingLatency::getAsCsv() const {
    return ""; // TODO
}

std::string MatchingEngineMonitor::OrderEventProcessingLatency::getAsTable() const {
    return ""; // TODO
}

MatchingEngineMonitor::MatchingEngineMonitor(const std::shared_ptr<Exchange::MatchingEngineBase>& matchingEngine) :
    myMatchingEngine(matchingEngine), myDebugMode(matchingEngine->isDebugMode()), myMonitoringEnabled(true) {
    if (!matchingEngine)
        Error::LIB_THROW("[MatchingEngineMonitor] Matching engine is null.");
    init();
    myMatchingEngine = matchingEngine;
    // add the callback to the matching engine once, and manage its lifetime internally via start/stopMonitoring()
    matchingEngine->addOrderProcessingCallback(myOrderProcessingCallback);
}

const MatchingEngineMonitor::OrderBookTopLevelsSnapshot& MatchingEngineMonitor::getLastOrderBookTopLevelsSnapshot() const {
    static const MatchingEngineMonitor::OrderBookTopLevelsSnapshot emptySnapshot;
    const auto& lastStats = myOrderBookStatisticsCollector.getLastSample();
    return lastStats ? lastStats->topLevelsSnapshot : emptySnapshot;
}

bool MatchingEngineMonitor::isPriceWithinTopOfBook(const Market::Side side, const double price, const std::optional<Market::OrderType>& type) const {
    if (type.has_value()) {
        if (*type == Market::OrderType::MARKET)
            return true;
    }
    const auto& topLevels = getLastOrderBookTopLevelsSnapshot();
    if (topLevels.isFullBook)
        return true;
    return (side == Market::Side::BUY && (topLevels.bidBookTopPrices.empty() || topLevels.bidBookTopPrices.size() < myOrderBookNumLevels || price >= topLevels.bidBookTopPrices.back())) ||
           (side == Market::Side::SELL && (topLevels.askBookTopPrices.empty() || topLevels.askBookTopPrices.size() < myOrderBookNumLevels || price <= topLevels.askBookTopPrices.back()));
}

void MatchingEngineMonitor::init() {
    myOrderBookStatisticsCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderEventProcessingLatenciesCollector.setMaxHistory(myTimeSeriesCollectorMaxSize);
    myOrderProcessingCallback = mySharedOrderProcessingCallback = std::make_shared<Exchange::OrderProcessingCallback>(
        [this](const std::shared_ptr<const Exchange::OrderProcessingReport>& report) {
            report->dispatchTo(*this);
        });
}

void MatchingEngineMonitor::startMonitoring() {
    myOrderProcessingCallback = mySharedOrderProcessingCallback;
    myMonitoringEnabled = true;
}

void MatchingEngineMonitor::stopMonitoring() {
    myOrderProcessingCallback = nullptr;
    myMonitoringEnabled = false;
}

void MatchingEngineMonitor::updateStatistics(const Exchange::OrderProcessingReport& report) {
    auto orderBookStats = std::make_shared<OrderBookStatisticsByTimestamp>(myOrderBookNumLevels, myFetchFullOrderBook);
    orderBookStats->constructFrom(myMatchingEngine, myOrderBookAggregateStatistics, myOrderBookAggregateStatisticsCache);
    myOrderBookStatisticsCollector.addSample(orderBookStats);
    myOrderProcessingReportsCollector.addSample(report.clone());
    myOrderBookAggregateStatisticsCache = myOrderBookAggregateStatistics;
    if (myDebugMode) {
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Added new order book statistics snapshot:\n" << orderBookStats->getAsTable();
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Updated order book aggregate statistics:\n" << myOrderBookAggregateStatistics.getAsTable();
    }
}

void MatchingEngineMonitor::exportToLobsterDataParser(Parser::LobsterDataParser& parser) const {
    if (myOrderProcessingReportsCollector.size() != myOrderBookStatisticsCollector.size())
        Error::LIB_THROW("[MatchingEngineMonitor::exportToLobsterDataParser] Mismatched number of order processing reports (" +
            std::to_string(myOrderProcessingReportsCollector.size()) + ") and order book statistics (" +
            std::to_string(myOrderBookStatisticsCollector.size()) + ").");
    const auto& reports = myOrderProcessingReportsCollector.getSamples();
    const auto& stats = myOrderBookStatisticsCollector.getSamples();
    auto reportsIt = reports.begin();
    auto statsIt = stats.begin();
    for (; reportsIt != reports.end() && statsIt != stats.end(); ++reportsIt, ++statsIt) {
        const auto& topLevelsSnapshot = (*statsIt)->topLevelsSnapshot;
        const auto message = (*reportsIt)->makeLobsterMessage();
        std::vector<uint32_t> bidPricesInt((*statsIt)->topLevelsSnapshot.bidBookTopPrices.size());
        std::vector<uint32_t> askPricesInt((*statsIt)->topLevelsSnapshot.askBookTopPrices.size());
        std::transform(
            (*statsIt)->topLevelsSnapshot.bidBookTopPrices.begin(),
            (*statsIt)->topLevelsSnapshot.bidBookTopPrices.end(),
            bidPricesInt.begin(), [](double p){ return Maths::castDoublePriceAsInt<uint32_t>(p); });
        std::transform(
            (*statsIt)->topLevelsSnapshot.askBookTopPrices.begin(),
            (*statsIt)->topLevelsSnapshot.askBookTopPrices.end(),
            askPricesInt.begin(), [](double p){ return Maths::castDoublePriceAsInt<uint32_t>(p); });
        auto snapshot = std::make_shared<const Parser::LobsterDataParser::OrderBookSnapshot>(
            bidPricesInt, askPricesInt, topLevelsSnapshot.bidBookTopSizes, topLevelsSnapshot.askBookTopSizes);
        if (message->isValid()) {
            parser.addOrderBookMessageAndSnapshot(message, snapshot);
        } else if (message->toSplitIntoDeleteAndAdd()) {
            // split order modify and cancel/replace report into atomic delete and add messages
            const auto atomMessages = (*reportsIt)->decomposeIntoAtomicReports();
            if (atomMessages.size() == 2 &&
                atomMessages[0]->orderProcessingType == Exchange::OrderProcessingType::CANCEL &&
                atomMessages[1]->orderProcessingType == Exchange::OrderProcessingType::SUBMIT) {
                const auto messageDel = atomMessages[0]->makeLobsterMessage();
                const auto messageAdd = atomMessages[1]->makeLobsterMessage();
                parser.addOrderBookMessageAndSnapshot(messageDel, nullptr);
                parser.addOrderBookMessageAndSnapshot(messageAdd, snapshot);
            }
        }
    }
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderExecutionReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order execution report received: " << report;
    if (myLastTrade && report.tradeId == myLastTrade->getId())
        return; // avoid double-counting execution reports for the same trade (sent twice from both sides)
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumTrades;
    myOrderBookAggregateStatistics.aggTradeVolume += report.filledQuantity;
    myOrderBookAggregateStatistics.aggTradeNotional += report.filledPrice * report.filledQuantity;
    myLastTrade = myMatchingEngine->getLastTrade();
    if (!myLastTrade->getIsBuyLimitOrder() || !myLastTrade->getIsSellLimitOrder())
        ++myOrderBookAggregateStatistics.aggNumNewMarketOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.filledPrice))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report) {
    // note that order submit report does NOT trigger statistics update since it only indicates that the matching engine
    // received the order but the actual order placement is signaled by the order placement report
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Limit order submit report received: " << report;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::LimitOrderPlacementReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Limit order placement report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumNewLimitOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice, Market::OrderType::LIMIT))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report) {
    // similar to limit order submit report, no statistics update triggered here
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Market order submit report received: " << report;
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order cancel report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice.value_or(Consts::NAN_DOUBLE), report.orderType))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderPartialCancelReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order partial cancel report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyQuantityOrders; // partial cancel represented as a modify quantity order
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice.value_or(Consts::NAN_DOUBLE), report.orderType))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order cancel and replace report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumCancelOrders;
    ++myOrderBookAggregateStatistics.aggNumNewLimitOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.newPrice, report.orderType))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order modify price report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyPriceOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.modifiedPrice))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}

void MatchingEngineMonitor::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report) {
    if (!myMonitoringEnabled)
        return;
    if (myDebugMode)
        *myLogger << Logger::LogLevel::DEBUG << "[MatchingEngineMonitor] Order modify quantity report received: " << report;
    myOrderBookAggregateStatistics.timestampTo = report.timestamp;
    ++myOrderBookAggregateStatistics.aggNumModifyQuantityOrders;
    if (myOrderBookStatisticsTimestampStrategy == OrderBookStatisticsTimestampStrategy::TOP_OF_BOOK_TICK) {
        if (!isPriceWithinTopOfBook(report.orderSide, report.orderPrice))
            return;
    } else
        Error::LIB_THROW("[MatchingEngineMonitor::onOrderProcessingReport] Unsupported order book statistics timestamp strategy: " + to_string(myOrderBookStatisticsTimestampStrategy));
    updateStatistics(report);
}
}

#endif
