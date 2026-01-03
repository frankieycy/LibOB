#ifndef MONITOR_OUTPUTS_ANALYZER_HPP
#define MONITOR_OUTPUTS_ANALYZER_HPP
#include "Utils/Utils.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

enum class MonitorOutputsFileFormat { LOBSTER, NONE };

class IMonitorOutputsAnalyzer {
public:
    IMonitorOutputsAnalyzer() = delete; // only permits construction from matching engine monitor
    IMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor);
    IMonitorOutputsAnalyzer(const std::string& monitorOutputsFilePath, const MonitorOutputsFileFormat fileFormat = MonitorOutputsFileFormat::LOBSTER);
    virtual ~IMonitorOutputsAnalyzer() = default;
    std::shared_ptr<const MatchingEngineMonitor> getMonitor() const { return myMonitor; }
    virtual void setMonitor(const std::shared_ptr<const MatchingEngineMonitor>& monitor) { myMonitor = monitor; }
    virtual void run() = 0;
private:
    std::shared_ptr<const MatchingEngineMonitor> myMonitor;
};

class MonitorOutputsAnalyzerBase : public IMonitorOutputsAnalyzer {
public:
    MonitorOutputsAnalyzerBase() = delete; // only permits construction from matching engine monitor
    MonitorOutputsAnalyzerBase(const std::shared_ptr<const MatchingEngineMonitor>& monitor) :
        IMonitorOutputsAnalyzer(monitor) {}
    MonitorOutputsAnalyzerBase(const std::string& monitorOutputsFilePath, const MonitorOutputsFileFormat fileFormat = MonitorOutputsFileFormat::LOBSTER) :
        IMonitorOutputsAnalyzer(monitorOutputsFilePath, fileFormat) {}
    virtual ~MonitorOutputsAnalyzerBase() = default;
    virtual void run() override {}
private:
    OrderDepthProfileStats myOrderDepthProfileStats;
    OrderFlowMemoryStats myOrderFlowMemoryStats;
    SpreadStats mySpreadStats;
};

std::string toString(const MonitorOutputsFileFormat& fileFormat);
std::ostream& operator<<(std::ostream& out, const MonitorOutputsFileFormat& fileFormat);
}

#endif
