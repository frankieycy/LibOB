#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

const std::string TEST_NAME = "MatchingEngineOrderCancelReplace";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    em.setPrintOrderBookPerOrderSubmit(true);
    em.setTimeEngineOrderEventsProcessing(true);
    em.submitLimitOrderEvent(Market::Side::BUY, 15, 99.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 5, 99.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 10, 98.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 5, 98.0);
    em.submitLimitOrderEvent(Market::Side::BUY, 10, 97.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 15, 102.0);
    em.submitLimitOrderEvent(Market::Side::SELL, 10, 103.0);
    // cancel-replace events
    em.cancelAndReplaceOrder(0, 10);
    em.cancelAndReplaceOrder(1, std::nullopt, 100.0);
    em.cancelAndReplaceOrder(2, 5, 97.0);
    em.cancelAndReplaceOrder(7, std::nullopt, 103.0);
    em.cancelAndReplaceOrder(8, 0);
    em.cancelAndReplaceOrder(12, 0);
    em.cancelAndReplaceOrder(4);
    em.cancelAndReplaceOrder(11);
    return 0;
}
