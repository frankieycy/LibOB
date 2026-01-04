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

void MonitorOutputsAnalyzerBase::clear() {
    // TODO
}

void MonitorOutputsAnalyzerBase::runAnalytics() {
    // TODO: implement analytic computations
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
    traces.orderBookStatisticsCollector = myMonitor->getOrderBookStatistics();
    traces.orderProcessingReportsCollector = myMonitor->getOrderProcessingReports();
}

LobsterDataParserOutputsAnalyzer::LobsterDataParserOutputsAnalyzer(const std::shared_ptr<const Parser::LobsterDataParser>& parser) :
    myParser(parser) {
    MonitorOutputsAnalyzerBase::init();
    init();
}

void LobsterDataParserOutputsAnalyzer::init() {
    // TODO
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
