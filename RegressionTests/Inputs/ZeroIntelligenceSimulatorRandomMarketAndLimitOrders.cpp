#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Simulator/ZeroIntelligence.hpp"

const std::string TEST_NAME = "ZeroIntelligenceSimulatorRandomMarketAndLimitOrders";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    zi->setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10 /* maxTimestamp */);
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.debugMode = true;
    simConfig.debugShowOrderBookPerEvent = true;
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 5);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 5);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 10, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(100.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    Parser::LobsterDataParser p;
    zi->getMatchingEngineMonitor()->exportToLobsterDataParser(p);
    const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
    *zi->getLogger() << "Lobster messages:\n" << data.first;
    *zi->getLogger() << "Lobster snapshots:\n" << data.second;
    return 0;
}
