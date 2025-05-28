#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

const std::string TEST_NAME = "MatchingEngineOrderEventManager";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false);
    em.setPrintOrderBookPerOrderSubmit(true);
    em.submitLimitOrderEvent(Market::Side::BUY, 15, 99.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 5, 99.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 10, 98.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 5, 98.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 10, 97.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 15, 102.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 103.0);
    em.submitMarketOrderEvent(Market::Side::BUY, 5);
    em.submitMarketOrderEvent(Market::Side::BUY, 15);
    em.submitMarketOrderEvent(Market::Side::BUY, 20);
    em.submitMarketOrderEvent(Market::Side::BUY, 15);
    em.submitMarketOrderEvent(Market::Side::SELL, 15);
    em.submitMarketOrderEvent(Market::Side::SELL, 25);
    em.submitMarketOrderEvent(Market::Side::SELL, 10);
    return 0;
}
