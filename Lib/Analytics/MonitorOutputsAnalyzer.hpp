#ifndef MONITOR_OUTPUTS_ANALYZER_HPP
#define MONITOR_OUTPUTS_ANALYZER_HPP
#include "Utils/Utils.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

enum class MonitorOutputsFileFormat { LOBSTER, NONE };

class MonitorOutputsAnalyzerBase {
public:
    MonitorOutputsAnalyzerBase() = delete; // only permits construction from matching engine monitor
    MonitorOutputsAnalyzerBase(const std::shared_ptr<const MatchingEngineMonitor>& monitor);
    MonitorOutputsAnalyzerBase(const std::string& monitorOutputsFilePath, const MonitorOutputsFileFormat fileFormat = MonitorOutputsFileFormat::LOBSTER);
    virtual ~MonitorOutputsAnalyzerBase() = default;
    std::shared_ptr<const MatchingEngineMonitor> getMonitor() const { return myMonitor; }
    void setMonitor(const std::shared_ptr<const MatchingEngineMonitor>& monitor) { myMonitor = monitor; }
private:
    std::shared_ptr<const MatchingEngineMonitor> myMonitor;
};
}

#endif
