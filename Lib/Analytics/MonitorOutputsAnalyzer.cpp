#ifndef MONITOR_OUTPUTS_ANALYZER_CPP
#define MONITOR_OUTPUTS_ANALYZER_CPP
#include "Utils/Utils.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

namespace Analytics {
using namespace Utils;

std::string toString(const IMonitorOutputsAnalyzer::OrderBookTracesSource& calcMode) {
    switch (calcMode) {
        case IMonitorOutputsAnalyzer::OrderBookTracesSource::MONITOR: return "Monitor";
        case IMonitorOutputsAnalyzer::OrderBookTracesSource::LOBSTER: return "Lobster";
        case IMonitorOutputsAnalyzer::OrderBookTracesSource::FILE:    return "File";
        default:                                                      return "None";
    }
}

std::string toString(const FileMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat) {
    switch (fileFormat) {
        case FileMonitorOutputsAnalyzer::MonitorOutputsFileFormat::LOBSTER: return "Lobster";
        default:                                                            return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const FileMonitorOutputsAnalyzer::OrderBookTracesSource& calcMode) { return out << toString(calcMode); }
std::ostream& operator<<(std::ostream& out, const FileMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat) { return out << toString(fileFormat); }

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
    myOrderDepthProfileStats.init();
    myOrderFlowMemoryStats.init();
    myPriceReturnScalingStats.init();
    mySpreadStats.init();
    myEventTimeStats.init();
    myOrderLifetimeStats.init();
}

void MonitorOutputsAnalyzerBase::clear() {
    myOrderDepthProfileStats.clear();
    myOrderFlowMemoryStats.clear();
    myPriceReturnScalingStats.clear();
    mySpreadStats.clear();
    myEventTimeStats.clear();
    myOrderLifetimeStats.clear();
}

void MonitorOutputsAnalyzerBase::runAnalytics() {
    // run analytic computations on order book traces
    const auto& traces = getOrderBookTraces();
    const auto& accumulationMode = getConfig().statsAccumulationMode;
    double bestBidPriceCache = Consts::NAN_DOUBLE;
    double bestAskPriceCache = Consts::NAN_DOUBLE;
    const auto& reports = traces.orderProcessingReportsCollector.getSamples();
    const auto& stats = traces.orderBookStatisticsCollector.getSamples();
    auto reportsIt = reports.begin();
    auto statsIt = stats.begin();
    for (; reportsIt != reports.end() && statsIt != stats.end(); ++reportsIt, ++statsIt) {
        const auto& report = *reportsIt;
        const auto& stat = *statsIt;
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
        myOrderLifetimeStats.accumulate(*stat, *report);
    }
    myOrderDepthProfileStats.compute();
    myOrderFlowMemoryStats.compute();
    myPriceReturnScalingStats.compute();
    mySpreadStats.compute();
    myEventTimeStats.compute();
    myOrderLifetimeStats.compute();
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
        << "\"OrderLifetimeStats\":"        << myOrderLifetimeStats.getAsJson()         << "\n"
        << "}";
    return oss.str();
}

MatchingEngineMonitorOutputsAnalyzer::MatchingEngineMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor) :
    myMonitor(monitor) {
    MonitorOutputsAnalyzerBase::init();
    init();
}

void MatchingEngineMonitorOutputsAnalyzer::init() {
    if (!myMonitor)
        Error::LIB_THROW("[MatchingEngineMonitorOutputsAnalyzer] Matching engine monitor is null.");
}

void MatchingEngineMonitorOutputsAnalyzer::populateOrderBookTraces() {
    // populate order book traces from monitor outputs
    auto& traces = getOrderBookTraces();
    if (myMonitor->getOrderBookStatistics().size() != myMonitor->getOrderProcessingReports().size())
        Error::LIB_THROW("[MatchingEngineMonitorOutputsAnalyzer::populateOrderBookTraces] Mismatched sizes between order book statistics and order processing reports in monitor.");
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
        Error::LIB_THROW("[LobsterDataParserOutputsAnalyzer] Lobster data parser is null.");
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
