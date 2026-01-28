#ifndef MONITOR_OUTPUTS_ANALYZER_CPP
#define MONITOR_OUTPUTS_ANALYZER_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

namespace Analytics {
using Utils::operator<<;

void MonitorOutputsAnalyzerBase::setStatsConfig(const OrderBookDerivedStatsConfig& config) {
    myStatsConfig = config;
    updateStatsConfig();
}

void MonitorOutputsAnalyzerBase::updateStatsConfig() {
    MonitorOutputsAnalyzerBase::init();
}

void MonitorOutputsAnalyzerBase::init() {
    myOrderDepthProfileStats.set(myStatsConfig.orderDepthProfileConfig);
    myOrderFlowMemoryStats.set(myStatsConfig.orderFlowMemoryStatsConfig);
    myPriceReturnScalingStats.set(myStatsConfig.priceReturnScalingStatsConfig);
    mySpreadStats.set(myStatsConfig.spreadStatsConfig);
    myEventTimeStats.set(myStatsConfig.eventTimeStatsConfig);
    myOrderLifetimeStats.set(myStatsConfig.orderLifetimeStatsConfig);
    myOrderImbalanceStats.set(myStatsConfig.orderImbalanceStatsConfig);
    myPriceImpactStats.set(myStatsConfig.priceImpactStatsConfig);
    myOrderDepthProfileStats.init();
    myOrderFlowMemoryStats.init();
    myPriceReturnScalingStats.init();
    mySpreadStats.init();
    myEventTimeStats.init();
    myOrderLifetimeStats.init();
    myOrderImbalanceStats.init();
    myPriceImpactStats.init();
}

void MonitorOutputsAnalyzerBase::clear() {
    myOrderDepthProfileStats.clear();
    myOrderFlowMemoryStats.clear();
    myPriceReturnScalingStats.clear();
    mySpreadStats.clear();
    myEventTimeStats.clear();
    myOrderLifetimeStats.clear();
    myOrderImbalanceStats.clear();
    myPriceImpactStats.clear();
}

void MonitorOutputsAnalyzerBase::runAnalytics() {
    // run analytic computations on order book traces
    const auto& traces = getOrderBookTraces();
    const auto& accumulationMode = getConfig().statsAccumulationMode;
    double bestBidPriceCache = Utils::Consts::NAN_DOUBLE;
    double bestAskPriceCache = Utils::Consts::NAN_DOUBLE;
    const auto& reports = traces.orderProcessingReportsCollector.getSamples();
    const auto& stats = traces.orderBookStatisticsCollector.getSamples();
    auto reportsIt = reports.begin();
    auto statsIt = stats.begin();
    for (; reportsIt != reports.end() && statsIt != stats.end(); ++reportsIt, ++statsIt) {
        const auto& report = *reportsIt;
        const auto& stat = *statsIt;
        if (!report || !stat)
            continue; // report and stat must be present at the same time
        // stats that need to be accumulated for every snapshot
        myOrderLifetimeStats.accumulate(*stat, report->orderSide);
        report->dispatchTo(*this); // report-specific accumulation (e.g. order births in lifetime stats)
        // stats that need to be accumulated conditionally based on accumulation mode
        bool accumulate = false;
        switch (accumulationMode) {
            case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::ALL:
                accumulate = true;
                break;
            case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::EACH_TRADE:
                accumulate = (stat->cumNumTrades > 0); // only when at least a trade has occurred
                break;
            case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::EACH_EVENT:
                accumulate = (stat->cumNumNewLimitOrders + stat->cumNumNewMarketOrders + stat->cumNumCancelOrders +
                    stat->cumNumModifyPriceOrders + stat->cumNumModifyQuantityOrders) > 0; // only when at least an event has occurred
                break;
            case MonitorOutputsAnalyzerConfig::OrderBookStatsAccumulationMode::LEVEL_ONE_TICK:
                accumulate = (stat->bestBidPrice != bestBidPriceCache) || (stat->bestAskPrice != bestAskPriceCache);
                bestBidPriceCache = stat->bestBidPrice;
                bestAskPriceCache = stat->bestAskPrice;
                break;
            default:
                break;
        }
        if (!accumulate)
            continue;
        myOrderDepthProfileStats.accumulate(stat->topLevelsSnapshot);
        myOrderFlowMemoryStats.accumulate(*stat->lastTradeIsBuyInitiated ? 1 : -1);
        myPriceReturnScalingStats.accumulate(*stat);
        mySpreadStats.accumulate(stat->spread);
        myEventTimeStats.accumulate(*stat);
        myOrderImbalanceStats.accumulate(*stat);
        myPriceImpactStats.accumulate(*stat);
    }
    myOrderDepthProfileStats.compute();
    myOrderFlowMemoryStats.compute();
    myPriceReturnScalingStats.compute();
    mySpreadStats.compute();
    myEventTimeStats.compute();
    myOrderLifetimeStats.compute();
    myOrderImbalanceStats.compute();
    myPriceImpactStats.compute();
}

std::string MonitorOutputsAnalyzerBase::getStatsReport() {
    std::ostringstream oss;
    oss << "{\n"
        << "\"StatsAccumulationMode\":\""   << getConfig().statsAccumulationMode        << "\",\n"
        << "\"OrderDepthProfileStats\":"    << myOrderDepthProfileStats.getAsJson()     << ",\n"
        << "\"OrderFlowMemoryStats\":"      << myOrderFlowMemoryStats.getAsJson()       << ",\n"
        << "\"PriceReturnScalingStats\":"   << myPriceReturnScalingStats.getAsJson()    << ",\n"
        << "\"SpreadStats\":"               << mySpreadStats.getAsJson()                << ",\n"
        << "\"EventTimeStats\":"            << myEventTimeStats.getAsJson()             << ",\n"
        << "\"OrderLifetimeStats\":"        << myOrderLifetimeStats.getAsJson()         << ",\n"
        << "\"OrderImbalanceStats\":"       << myOrderImbalanceStats.getAsJson()        << ",\n"
        << "\"PriceImpactStats\":"          << myPriceImpactStats.getAsJson()           << "\n"
        << "}";
    return oss.str();
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderExecutionReport& report) {
    // Note that the matching engine sends out execution reports for both (incoming) taker and (resting) maker orders, and
    // the matching engine monitor logs only the taker order (i.e. market order side). Effectively, only the "!isMakerOrder"
    // block is executed in practice.
    if (report.isMakerOrder) // execution on resting limit order hence this order id
        myOrderLifetimeStats.onOrderExecute(report.orderId, report.filledQuantity);
    else // execution on taker market order hence match order id
        myOrderLifetimeStats.onOrderExecute(report.matchOrderId, report.filledQuantity);
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& /* report */) {
    // no implementation needed yet
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::LimitOrderPlacementReport& report) {
    myOrderLifetimeStats.onOrderPlacement(report.orderId, report.orderSide, report.orderQuantity, report.orderPrice);
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& /* report */) {
    // no implementation needed yet
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderCancelReport& report) {
    if (report.orderType == Market::OrderType::LIMIT)
        myOrderLifetimeStats.onOrderCancel(report.orderId);
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderPartialCancelReport& report) {
    if (report.orderType == Market::OrderType::LIMIT)
        myOrderLifetimeStats.onOrderPartialCancel(report.orderId, report.cancelQuantity);
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& /* report */) {
    // no implementation needed yet
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderModifyPriceReport& /* report */) {
    // no implementation needed yet
}

void MonitorOutputsAnalyzerBase::onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& /* report */) {
    // no implementation needed yet
}


MatchingEngineMonitorOutputsAnalyzer::MatchingEngineMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor) :
    myMonitor(monitor) {
    MonitorOutputsAnalyzerBase::init();
    init();
}

void MatchingEngineMonitorOutputsAnalyzer::init() {
    if (!myMonitor)
        Utils::Error::LIB_THROW("[MatchingEngineMonitorOutputsAnalyzer] Matching engine monitor is null.");
}

void MatchingEngineMonitorOutputsAnalyzer::populateOrderBookTraces() {
    // populate order book traces from monitor outputs
    auto& traces = getOrderBookTraces();
    if (myMonitor->getOrderBookStatistics().size() != myMonitor->getOrderProcessingReports().size())
        Utils::Error::LIB_THROW("[MatchingEngineMonitorOutputsAnalyzer::populateOrderBookTraces] Mismatched sizes between order book statistics and order processing reports in monitor.");
    traces.orderBookStatisticsCollector = myMonitor->getOrderBookStatistics(); // owns a deep copy of the time series
    traces.orderProcessingReportsCollector = myMonitor->getOrderProcessingReports();
}

LobsterDataParserOutputsAnalyzer::LobsterDataParserOutputsAnalyzer(const std::shared_ptr<const Parser::LobsterDataParser>& parser) :
    myParser(parser) {
    MonitorOutputsAnalyzerBase::init();
    init();
}

void LobsterDataParserOutputsAnalyzer::init() {
    if (!myParser)
        Utils::Error::LIB_THROW("[LobsterDataParserOutputsAnalyzer] Lobster data parser is null.");
}

void LobsterDataParserOutputsAnalyzer::populateOrderBookTraces() {
    // TODO: populate order book traces from lobster parser outputs
}

FileMonitorOutputsAnalyzer::FileMonitorOutputsAnalyzer(const std::string& filePath, const MonitorOutputsFileFormat fileFormat) :
    myFilePath(filePath), myFileFormat(fileFormat) {
    MonitorOutputsAnalyzerBase::init();
    init();
}

void FileMonitorOutputsAnalyzer::init() {
    // TODO
}

void FileMonitorOutputsAnalyzer::populateOrderBookTraces() {
    // TODO: populate order book traces from file monitor outputs
}
}

#endif
