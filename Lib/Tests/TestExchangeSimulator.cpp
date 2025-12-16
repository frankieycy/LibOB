#ifndef TEST_EXCHANGE_SIMULATOR_CPP
#define TEST_EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Tests/TestExchangeSimulator.hpp"

namespace Tests {
namespace Simulator {
void testInitExchangeSimulatorBase() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
}

void testInitZeroIntelligenceSimulator() {
    // TODO
}
}
}

#endif
