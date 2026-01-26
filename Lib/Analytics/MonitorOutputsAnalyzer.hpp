#ifndef MONITOR_OUTPUTS_ANALYZER_HPP
#define MONITOR_OUTPUTS_ANALYZER_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Parser/LobsterDataParser.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Analytics/OrderBookDerivedAnalytics.hpp"
#include "Analytics/OrderBookDerivedAnalyticsUtils.hpp"

namespace Analytics {
using namespace Utils;

/* Performs analytics on matching engine monitor outputs e.g. top-level order book snapshots.
    Supports construction from matching engine monitor, lobster data parser and outputs file path, */
class IMonitorOutputsAnalyzer {
public:
    enum class OrderBookTracesSource { MONITOR, LOBSTER, FILE, NONE };
    bool isDebugMode() const { return myConfig.debugMode; }
    MonitorOutputsAnalyzerConfig& getConfig() { return myConfig; }
    const MonitorOutputsAnalyzerConfig& getConfig() const { return myConfig; }
    OrderBookTraces& getOrderBookTraces() { return myOrderBookTraces; }
    const OrderBookTraces& getOrderBookTraces() const { return myOrderBookTraces; }
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void setDebugMode(const bool debugMode) { myConfig.debugMode = debugMode; }
    virtual void setConfig(const MonitorOutputsAnalyzerConfig& config) { myConfig = config; }
    virtual void setOrderBookTraces(const OrderBookTraces& traces) { myOrderBookTraces = traces; }
    virtual void populateOrderBookTraces() = 0; // populates internal order book traces from monitor outputs
    virtual void runAnalytics() = 0; // runs analytics on the populated book traces
    virtual std::string getStatsReport() = 0; // returns a string report of the computed statistics
    static constexpr OrderBookTracesSource ourSource = OrderBookTracesSource::NONE;

private:
    MonitorOutputsAnalyzerConfig myConfig = MonitorOutputsAnalyzerConfig();
    OrderBookTraces myOrderBookTraces = OrderBookTraces();
};

class MonitorOutputsAnalyzerBase : public IMonitorOutputsAnalyzer {
public:
    OrderBookDerivedStatsConfig& getStatsConfig() { return myStatsConfig; }
    const OrderBookDerivedStatsConfig& getStatsConfig() const { return myStatsConfig; }
    virtual void setStatsConfig(const OrderBookDerivedStatsConfig& config);
    virtual void updateStatsConfig(); // updates internal analytic components with the current stats config
    virtual void init() override;
    virtual void clear() override;
    virtual void runAnalytics() override;
    virtual std::string getStatsReport() override;

    // dispatchers of specific reports into the analytic components
    virtual void onOrderProcessingReport(const Exchange::OrderExecutionReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderPlacementReport& report);
    virtual void onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderPartialCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelAndReplaceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report);

private:
    OrderBookDerivedStatsConfig myStatsConfig = OrderBookDerivedStatsConfig();
    // define all the analytic components here
    OrderDepthProfileStats myOrderDepthProfileStats = OrderDepthProfileStats();
    OrderFlowMemoryStats myOrderFlowMemoryStats = OrderFlowMemoryStats();
    PriceReturnScalingStats myPriceReturnScalingStats = PriceReturnScalingStats();
    SpreadStats mySpreadStats = SpreadStats();
    EventTimeStats myEventTimeStats = EventTimeStats();
    OrderLifetimeStats myOrderLifetimeStats = OrderLifetimeStats();
    OrderImbalanceStats myOrderImbalanceStats = OrderImbalanceStats();
    PriceImpactStats myPriceImpactStats = PriceImpactStats();
};

class MatchingEngineMonitorOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    MatchingEngineMonitorOutputsAnalyzer() = delete; // only permits construction from monitor
    MatchingEngineMonitorOutputsAnalyzer(const std::shared_ptr<const MatchingEngineMonitor>& monitor);
    virtual ~MatchingEngineMonitorOutputsAnalyzer() = default;
    std::shared_ptr<const MatchingEngineMonitor> getMonitor() const { return myMonitor; }
    virtual void init() override;
    virtual void populateOrderBookTraces() override;
    static constexpr OrderBookTracesSource ourSource = OrderBookTracesSource::MONITOR;

private:
    std::shared_ptr<const MatchingEngineMonitor> myMonitor;
};

class LobsterDataParserOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    LobsterDataParserOutputsAnalyzer() = delete; // only permits construction from lobster data parser
    LobsterDataParserOutputsAnalyzer(const std::shared_ptr<const Parser::LobsterDataParser>& parser);
    virtual ~LobsterDataParserOutputsAnalyzer() = default;
    std::shared_ptr<const Parser::LobsterDataParser> getParser() const { return myParser; }
    virtual void init() override;
    virtual void populateOrderBookTraces() override;
    static constexpr OrderBookTracesSource ourSource = OrderBookTracesSource::LOBSTER;

private:
    std::shared_ptr<const Parser::LobsterDataParser> myParser;
};

class FileMonitorOutputsAnalyzer : public MonitorOutputsAnalyzerBase {
public:
    enum class MonitorOutputsFileFormat { LOBSTER, NONE };
    FileMonitorOutputsAnalyzer() = delete; // only permits construction from file path
    FileMonitorOutputsAnalyzer(const std::string& filePath, const MonitorOutputsFileFormat fileFormat);
    virtual ~FileMonitorOutputsAnalyzer() = default;
    std::string getFilePath() const { return myFilePath; }
    MonitorOutputsFileFormat getFileFormat() const { return myFileFormat; }
    virtual void init() override;
    virtual void populateOrderBookTraces() override;
    static constexpr OrderBookTracesSource ourSource = OrderBookTracesSource::FILE;

private:
    std::string myFilePath;
    MonitorOutputsFileFormat myFileFormat = MonitorOutputsFileFormat::NONE;
};
}

template<>
struct Utils::EnumStrings<Analytics::IMonitorOutputsAnalyzer::OrderBookTracesSource> {
    inline static constexpr std::array<const char*, 4> names = { "Monitor", "Lobster", "File", "None" };
};

template<>
struct Utils::EnumStrings<Analytics::FileMonitorOutputsAnalyzer::MonitorOutputsFileFormat> {
    inline static constexpr std::array<const char*, 2> names = { "Lobster", "None" };
};

#endif
