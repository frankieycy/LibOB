#ifndef MONITOR_OUTPUTS_ANALYZER_CPP
#define MONITOR_OUTPUTS_ANALYZER_CPP
#include "Utils/Utils.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

namespace Analytics {
using namespace Utils;

MonitorOutputsAnalyzerBase::MonitorOutputsAnalyzerBase(const std::shared_ptr<const MatchingEngineMonitor>& monitor) :
    myMonitor(monitor) {
    if (!monitor)
        Error::LIB_THROW("[MonitorOutputsAnalyzerBase] Matching engine monitor is null.");
}

MonitorOutputsAnalyzerBase::MonitorOutputsAnalyzerBase(const std::string& /* monitorOutputsFilePath */, const MonitorOutputsFileFormat /* fileFormat */) :
    myMonitor(nullptr) {
    // TODO: load the monitor outputs from file path
}
}

#endif
