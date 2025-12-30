#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Simulator/ZeroIntelligence.hpp"

const std::string TEST_NAME = "ZeroIntelligenceSimulatorRandomMarketOrders";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    zi->setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10 /* maxTimestamp */);
    // simulator-level config
    Simulator::ExchangeSimulatorConfig simConfig(zi->getConfig());
    simConfig.debugMode = true;
    simConfig.debugShowOrderBookPerEvent = true;
    // zero-intelligence config
    Simulator::ZeroIntelligenceConfig ziConfig(zi->getZIConfig());
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 10);
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 20, 2, 40), // linear interp from 2 @ 1 tick ($0.01) to 40 @ 20 ticks ($0.20)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(20, 40), // flat extrap at 40 beyond 20 ticks
        40);
    zi->setConfig(simConfig); // set the general configs first before altering the specifics with an exposed interface
    zi->setZIConfig(ziConfig);
    zi->setAnchorPrice(10.00);
    zi->setMinPriceTick(0.01);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $0.2 around the $10 anchor
    zi->simulate();
    return 0;
}
