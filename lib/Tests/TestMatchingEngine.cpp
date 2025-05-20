#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Tests/TestMatchingEngine.hpp"

namespace Tests {
namespace MatchingEngine {
void testMatchingEngineSimpleBook() {
    uint8_t orderId = 0;
    uint8_t timestamp = 0;
    Exchange::MatchingEngineFIFO e{true};
    const std::vector<const std::shared_ptr<Market::OrderBase>> orders{
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::BUY, 15, 99.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::BUY, 5, 99.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::BUY, 10, 98.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::BUY, 5, 98.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::BUY, 10, 97.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::SELL, 10, 101.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::SELL, 10, 101.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::SELL, 15, 102.0),
        std::make_shared<Market::LimitOrder>(orderId++, timestamp++, Market::Side::SELL, 10, 103.0),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::BUY, 5),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::BUY, 15),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::BUY, 20),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::BUY, 15),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::SELL, 15),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::SELL, 25),
        std::make_shared<Market::MarketOrder>(orderId++, timestamp++, Market::Side::SELL, 10),
    };
    for (const auto& o : orders) {
        e.process(o);
        std::cout << e << std::endl;
    }
}

void testMatchingEngineOrderEventManager() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
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
}

void testMatchingEngineRandomOrders() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
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
        const uint32_t ui = Utils::Statistics::getRandomUniformInt(1, 3, true);
        if (u0 < 0.5) {
            if (u1 < 0.5)
                em.submitLimitOrderEvent(Market::Side::BUY, ui, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Market::Side::SELL, ui, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Market::Side::BUY, ui);
            else
                em.submitMarketOrderEvent(Market::Side::SELL, ui);
        }
    }
    // random orders in bulk
    em.setDebugMode(false);
    for (int i = 0; i < 100; ++i) {
        const double u0 = Utils::Statistics::getRandomUniform01(true);
        const double u1 = Utils::Statistics::getRandomUniform01(true);
        const int ui = Utils::Statistics::getRandomUniformInt(1, 3, true);
        if (u0 < 0.5) {
            if (u1 < 0.5)
                em.submitLimitOrderEvent(Market::Side::BUY, ui, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Market::Side::SELL, ui, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Market::Side::BUY, ui);
            else
                em.submitMarketOrderEvent(Market::Side::SELL, ui);
        }
    }
    Utils::IO::printDebugBanner(std::cout);
    std::cout << *e << std::endl;
    Utils::IO::printDebugBanner(std::cout);
    std::cout << em << std::endl;
}

void testMatchingEngineOrderCancelModify() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
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
    em.modifyOrderPrice(7, 103.0);
    em.modifyOrderPrice(9, 100.0);
    // TODO: modify quantity
}

void testMatchingEngineZeroIntelligence() {
    // TODO
}
}
}
