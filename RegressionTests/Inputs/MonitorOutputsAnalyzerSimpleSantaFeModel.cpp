#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Simulator/ZeroIntelligence.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"

const std::string TEST_NAME = "MonitorOutputsAnalyzerSimpleSantaFeModel";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    std::shared_ptr<Analytics::MatchingEngineMonitorOutputsAnalyzer> a = std::make_shared<Analytics::MatchingEngineMonitorOutputsAnalyzer>(zi->getMatchingEngineMonitor());
    zi->setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10000 /* maxTimestamp */);
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(10.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(30.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelRateSampler = std::make_shared<Simulator::OrderEventRateSamplerProportionalTotalSizeFromOppositeBest>(0.2, 1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSideSampler = std::make_shared<Simulator::OrderSideSamplerProportionalTotalSizeFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSampler = std::make_shared<Simulator::OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest>(1, 1, 30, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(1000.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    auto& statsConfig = a->getStatsConfig();
    statsConfig.orderDepthProfileConfig.minPriceTick = zi->getMinPriceTick(); // 1.0
    statsConfig.orderDepthProfileConfig.maxTicks = zi->getMonitoredOrderBookNumLevels(); // 100
    statsConfig.orderLifetimeStatsConfig.minPriceTick = zi->getMinPriceTick(); // 1.0
    statsConfig.orderLifetimeStatsConfig.maxTicks = zi->getMonitoredOrderBookNumLevels(); // 100
    statsConfig.orderImbalanceStatsConfig.minPriceTick = zi->getMinPriceTick(); // 1.0
    statsConfig.orderImbalanceStatsConfig.maxTicks = zi->getMonitoredOrderBookNumLevels(); // 100
    statsConfig.priceImpactStatsConfig.minPriceTick = zi->getMinPriceTick(); // 1.0
    statsConfig.spreadStatsConfig.maxSpread = 100.0 * zi->getMinPriceTick(); // 100.0
    statsConfig.spreadStatsConfig.numBins = 100; // 100 bins over $0 to $100
    a->updateStatsConfig();
    a->populateOrderBookTraces();
    a->runAnalytics();
    *zi->getLogger() << "Monitor outputs analytics:\n" << a->getStatsReport();
    return 0;
}
