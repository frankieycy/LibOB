#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

const std::string TEST_NAME = "MatchingEngineRandomOrders";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
    // initial book state
    for (int i = 0; i < 20; ++i) {
        em.submitLimitOrderEvent(Market::Side::BUY, std::min(5 + i, 10), 99.0 - i);
        em.submitLimitOrderEvent(Market::Side::SELL, std::min(5 + i, 10), 101.0 + i);
    }
    // random limit and market orders
    em.setPrintOrderBookPerOrderSubmit(true);
    for (int i = 0; i < 10; ++i) {
        const double u0 = Utils::Statistics::getRandomUniform01(true);
        const double u1 = Utils::Statistics::getRandomUniform01(true);
        const uint32_t qty = Utils::Statistics::getRandomUniformInt(1, 3, true);
        if (u0 < 0.5) {
            if (u1 < 0.5)
                em.submitLimitOrderEvent(Market::Side::BUY, qty, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Market::Side::SELL, qty, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Market::Side::BUY, qty);
            else
                em.submitMarketOrderEvent(Market::Side::SELL, qty);
        }
    }
    // random orders in bulk
    em.setDebugMode(false);
    for (int i = 0; i < 100; ++i) {
        const double u0 = Utils::Statistics::getRandomUniform01(true);
        const double u1 = Utils::Statistics::getRandomUniform01(true);
        const int qty = Utils::Statistics::getRandomUniformInt(1, 3, true);
        if (u0 < 0.5) {
            if (u1 < 0.5)
                em.submitLimitOrderEvent(Market::Side::BUY, qty, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Market::Side::SELL, qty, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Market::Side::BUY, qty);
            else
                em.submitMarketOrderEvent(Market::Side::SELL, qty);
        }
    }
    *em.getLogger() << "Order book state:\n" << *e;
    *em.getLogger() << "Order event manager state:\n" << em;
    return 0;
}
