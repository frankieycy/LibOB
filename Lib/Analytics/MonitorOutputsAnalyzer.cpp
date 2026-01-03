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

IMonitorOutputsAnalyzer::IMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor) :
    myMonitor(monitor),
    myCalculationMode(CalculationMode::FROM_MONITOR) {
    if (!monitor)
        Error::LIB_THROW("[IMonitorOutputsAnalyzer] Matching engine monitor is null.");
}

IMonitorOutputsAnalyzer::IMonitorOutputsAnalyzer(const std::string& /* monitorOutputsFilePath */, const MonitorOutputsFileFormat /* fileFormat */) :
    myMonitor(nullptr),
    myCalculationMode(CalculationMode::FROM_FILE) {
    // TODO: load the monitor outputs from file path
}
}

#endif
