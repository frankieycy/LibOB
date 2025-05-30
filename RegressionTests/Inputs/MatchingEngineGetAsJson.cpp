#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

const std::string TEST_NAME = "MatchingEngineGetAsJson";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    // initial book state
    for (int i = 0; i < 20; ++i) {
        em.submitLimitOrderEvent(Market::Side::BUY, std::min(5 + i, 10), 99.0 - i);
        em.submitLimitOrderEvent(Market::Side::SELL, std::min(5 + i, 10), 101.0 + i);
    }
    // get matching engine state as Json
    *em.getLogger() << "Matching engine Json:\n" << e->getAsJson();
    return 0;
}
