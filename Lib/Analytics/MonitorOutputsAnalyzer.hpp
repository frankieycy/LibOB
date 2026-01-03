#ifndef MONITOR_OUTPUTS_ANALYZER_HPP
#define MONITOR_OUTPUTS_ANALYZER_HPP
#include "Utils/Utils.hpp"
#include "Parser/LobsterDataParser.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"

namespace Analytics {
using namespace Utils;

/* Performs analytics on matching engine monitor outputs e.g. top-level order book snapshots.
    Supports construction from matching engine monitor, lobster data parser and outputs file path, */
class IMonitorOutputsAnalyzer {
public:
    enum class CalculationMode { FROM_MONITOR, FROM_LOBSTER, FROM_FILE, NONE };
    enum class MonitorOutputsFileFormat { LOBSTER, NONE };
    OrderBookTraces& getOrderBookTraces() { return myOrderBookTraces; }
    const OrderBookTraces& getOrderBookTraces() const { return myOrderBookTraces; }
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void setOrderBookTraces(const OrderBookTraces& traces) { myOrderBookTraces = traces; }
    virtual void populateOrderBookTraces() = 0; // populates internal order book traces from monitor outputs
    virtual void runAnalytics() = 0; // runs analytics on the populated book traces
    static constexpr CalculationMode ourCalcMode = CalculationMode::NONE;
private:
    OrderBookTraces myOrderBookTraces;
};

class MonitorOutputsAnalyzerBase : public IMonitorOutputsAnalyzer {
public:
    virtual void init() override;
    virtual void clear() override;
    virtual void runAnalytics() override;
private:
    // define all the analytic components here
    OrderDepthProfileStats myOrderDepthProfileStats;
    OrderFlowMemoryStats myOrderFlowMemoryStats;
    SpreadStats mySpreadStats;
};

class MatchingEngineMonitorOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    MatchingEngineMonitorOutputsAnalyzer() = delete; // only permits construction from monitor
    MatchingEngineMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor) : myMonitor(monitor) {}
    virtual ~MatchingEngineMonitorOutputsAnalyzer() = default;
    std::shared_ptr<const MatchingEngineMonitor> getMonitor() const { return myMonitor; }
    virtual void populateOrderBookTraces() override;
    static constexpr CalculationMode ourCalcMode = CalculationMode::FROM_MONITOR;
private:
    std::shared_ptr<const MatchingEngineMonitor> myMonitor;
};

class LobsterDataParserOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    LobsterDataParserOutputsAnalyzer() = delete; // only permits construction from lobster data parser
    LobsterDataParserOutputsAnalyzer(const std::shared_ptr<const Parser::LobsterDataParser>& parser) : myParser(parser) {}
    virtual ~LobsterDataParserOutputsAnalyzer() = default;
    std::shared_ptr<const Parser::LobsterDataParser> getParser() const { return myParser; }
    virtual void populateOrderBookTraces() override;
    static constexpr CalculationMode ourCalcMode = CalculationMode::FROM_LOBSTER;
private:
    std::shared_ptr<const Parser::LobsterDataParser> myParser;
};

class FileMonitorOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    FileMonitorOutputsAnalyzer() = delete; // only permits construction from file path
    FileMonitorOutputsAnalyzer(const std::string& filePath, const MonitorOutputsFileFormat fileFormat) :
        myFilePath(filePath), myFileFormat(fileFormat) {}
    virtual ~FileMonitorOutputsAnalyzer() = default;
    std::string getFilePath() const { return myFilePath; }
    MonitorOutputsFileFormat getFileFormat() const { return myFileFormat; }
    virtual void populateOrderBookTraces() override;
    static constexpr CalculationMode ourCalcMode = CalculationMode::FROM_FILE;
private:
    std::string myFilePath;
    MonitorOutputsFileFormat myFileFormat;
};

std::string toString(const IMonitorOutputsAnalyzer::CalculationMode& calcMode);
std::string toString(const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat);
std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::CalculationMode& calcMode);
std::ostream& operator<<(std::ostream& out, const IMonitorOutputsAnalyzer::MonitorOutputsFileFormat& fileFormat);
}

#endif
