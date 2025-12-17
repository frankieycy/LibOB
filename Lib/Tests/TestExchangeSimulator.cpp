#ifndef TEST_EXCHANGE_SIMULATOR_CPP
#define TEST_EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Simulator/ZeroIntelligence.hpp"
#include "Tests/TestExchangeSimulator.hpp"

namespace Tests {
namespace ExchangeSimulator {
void testInitExchangeSimulatorBase() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
}

void testInitZeroIntelligenceSimulator() {
    // TODO
}
}
}

#endif
