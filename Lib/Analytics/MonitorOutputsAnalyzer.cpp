#ifndef MONITOR_OUTPUTS_ANALYZER_CPP
#define MONITOR_OUTPUTS_ANALYZER_CPP
#include "Utils/Utils.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

namespace Analytics {
using namespace Utils;

std::string toString(const IMonitorOutputsAnalyzer::CalculationMode& calcMode) {
    switch (calcMode) {
        case IMonitorOutputsAnalyzer::CalculationMode::FROM_MONITOR: return "FromMonitor";
        case IMonitorOutputsAnalyzer::CalculationMode::FROM_FILE:    return "FromFile";
        default:                                                     return "None";
    }
}

std::string toString(const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat) {
    switch (fileFormat) {
        case IMonitorOutputsAnalyzer::MonitorOutputsFileFormat::LOBSTER: return "Lobster";
        default:                                                         return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::CalculationMode& calcMode) { return out << toString(calcMode); }
std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat) { return out << toString(fileFormat); }

void MonitorOutputsAnalyzerBase::init() {
    // TODO
}

void MonitorOutputsAnalyzerBase::clear() {
    // TODO
}

void MonitorOutputsAnalyzerBase::runAnalytics() {
    // TODO: implement analytic computations
}

void MatchingEngineMonitorOutputsAnalyzer::populateOrderBookTraces() {
    // TODO: populate order book traces from monitor outputs
}

void LobsterDataParserOutputsAnalyzer::populateOrderBookTraces() {
    // TODO: populate order book traces from lobster parser outputs
}

void FileMonitorOutputsAnalyzer::populateOrderBookTraces() {
    // TODO: populate order book traces from file monitor outputs
}
}

#endif
