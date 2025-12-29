#ifndef TEST_EXCHANGE_SIMULATOR_CPP
#define TEST_EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Simulator/VolumeProfile.hpp"
#include "Simulator/ZeroIntelligence.hpp"
#include "Tests/TestExchangeSimulator.hpp"

namespace Tests {
namespace ExchangeSimulator {
void testInitZeroIntelligenceSimulator() {
    std::shared_ptr<Exchange::IMatchingEngine> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::IExchangeSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 20, 2, 40), // linear interp from 2 @ 1 tick ($0.01) to 40 @ 20 ticks ($0.20)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(20, 40), // flat extrap at 40 beyond 20 ticks
        40);
    zi->setAnchorPrice(10.00);
    zi->setMinPriceTick(0.01);
    zi->initOrderBookBuilding(v0, v0); // linear book of $0.2 around the $10 anchor
    auto& config = e->getOrderBookDisplayConfig();
    config.setPrintAsciiOrderBook(true);
    config.setOrderBookLevels(30);
    std::cout << *e << std::endl;
}

void testZeroIntelligenceSimulatorRandomOrders() {
    std::shared_ptr<Exchange::IMatchingEngine> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::IExchangeSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10);
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 20, 2, 40), // linear interp from 2 @ 1 tick ($0.01) to 40 @ 20 ticks ($0.20)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(20, 40), // flat extrap at 40 beyond 20 ticks
        40);
    zi->setAnchorPrice(10.00);
    zi->setMinPriceTick(0.01);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $0.2 around the $10 anchor
    zi->simulate();
}
}
}

#endif
