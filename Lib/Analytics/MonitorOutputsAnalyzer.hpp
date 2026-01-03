#ifndef MONITOR_OUTPUTS_ANALYZER_HPP
#define MONITOR_OUTPUTS_ANALYZER_HPP
#include "Utils/Utils.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;
class IMonitorOutputsAnalyzer {
public:
    enum class CalculationMode { FROM_MONITOR, FROM_FILE, NONE };
    enum class MonitorOutputsFileFormat { LOBSTER, NONE };
    IMonitorOutputsAnalyzer() = delete; // only permits construction from matching engine monitor
    IMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor);
    IMonitorOutputsAnalyzer(const std::string& monitorOutputsFilePath, const MonitorOutputsFileFormat fileFormat = MonitorOutputsFileFormat::LOBSTER);
    virtual ~IMonitorOutputsAnalyzer() = default;
    std::shared_ptr<const MatchingEngineMonitor> getMonitor() const { return myMonitor; }
    CalculationMode getCalculationMode() const { return myCalculationMode; }
    virtual void setMonitor(const std::shared_ptr<const MatchingEngineMonitor>& monitor) { myMonitor = monitor; }
    virtual void setCalculationMode(const CalculationMode calcMode) { myCalculationMode = calcMode; }
    virtual void run() = 0;
private:
    std::shared_ptr<const MatchingEngineMonitor> myMonitor;
    CalculationMode myCalculationMode = CalculationMode::NONE;
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

std::string toString(const IMonitorOutputsAnalyzer::CalculationMode& calcMode);
std::string toString(const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat);
std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::CalculationMode& calcMode);
std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat);
}

#endif
