#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

const std::string TEST_NAME = "MatchingEngineOrderCancelModify";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
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
    // cancel
    em.cancelOrder(0);
    em.cancelOrder(1);
    em.cancelOrder(8);
    em.cancelOrder(9);
    // modify price
    em.modifyOrderPrice(2, 100.0);
    em.modifyOrderPrice(4, 95.0);
    em.modifyOrderPrice(5, 102.0);
    em.modifyOrderPrice(7, 102.0);
    em.modifyOrderPrice(7, 103.0);
    em.modifyOrderPrice(9, 100.0);
    // modify quantity
    em.modifyOrderQuantity(2, 5);
    em.modifyOrderQuantity(3, 20);
    em.modifyOrderQuantity(4, 0);
    em.modifyOrderQuantity(6, 0);
    em.submitLimitOrderEvent(Market::Side::SELL, 5, 103.0);
    em.modifyOrderQuantity(7, 15);
    em.modifyOrderQuantity(10, 10);
    return 0;
}
