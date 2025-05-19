#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Tests {
namespace MatchingEngine {
using namespace Market;
using namespace Exchange;

void testMatchingEngineSimpleBook() {
    uint8_t orderId = 0;
    uint8_t timestamp = 0;
    MatchingEngineFIFO e{true};
    const std::vector<const std::shared_ptr<OrderBase>> orders{
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::BUY, 15, 99.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::BUY, 5, 99.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::BUY, 10, 98.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::BUY, 5, 98.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::BUY, 10, 97.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::SELL, 10, 101.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::SELL, 10, 101.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::SELL, 15, 102.0),
        std::make_shared<LimitOrder>(orderId++, timestamp++, Side::SELL, 10, 103.0),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::BUY, 5),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::BUY, 15),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::BUY, 20),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::BUY, 15),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::SELL, 15),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::SELL, 25),
        std::make_shared<MarketOrder>(orderId++, timestamp++, Side::SELL, 10),
    };
    for (const auto& o : orders) {
        e.process(o);
        std::cout << e << std::endl;
    }
}

void testMatchingEngineOrderEventManager() {
    std::shared_ptr<MatchingEngineFIFO> e = std::make_shared<MatchingEngineFIFO>(true);
    OrderEventManagerBase em{e};
    em.setPrintOrderBookPerOrderSubmit(true);
    em.submitLimitOrderEvent(Side::BUY, 15, 99.0);
    em.submitLimitOrderEvent(Side::BUY, 5, 99.0);
    em.submitLimitOrderEvent(Side::BUY, 10, 98.0);
    em.submitLimitOrderEvent(Side::BUY, 5, 98.0);
    em.submitLimitOrderEvent(Side::BUY, 10, 97.0);
    em.submitLimitOrderEvent(Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Side::SELL, 10, 101.0);
    em.submitLimitOrderEvent(Side::SELL, 15, 102.0);
    em.submitLimitOrderEvent(Side::SELL, 10, 103.0);
    em.submitMarketOrderEvent(Side::BUY, 5);
    em.submitMarketOrderEvent(Side::BUY, 15);
    em.submitMarketOrderEvent(Side::BUY, 20);
    em.submitMarketOrderEvent(Side::BUY, 15);
    em.submitMarketOrderEvent(Side::SELL, 15);
    em.submitMarketOrderEvent(Side::SELL, 25);
    em.submitMarketOrderEvent(Side::SELL, 10);
}

void testMatchingEngineRandomOrders() {
    std::shared_ptr<MatchingEngineFIFO> e = std::make_shared<MatchingEngineFIFO>(true);
    OrderEventManagerBase em{e};
    // initial book state
    for (int i = 0; i < 20; ++i) {
        em.submitLimitOrderEvent(Side::BUY, std::min(5 + i, 10), 99.0 - i);
        em.submitLimitOrderEvent(Side::SELL, std::min(5 + i, 10), 101.0 + i);
    }
    // random limit and market orders
    em.setPrintOrderBookPerOrderSubmit(true);
    for (int i = 0; i < 10; ++i) {
        const double u0 = Utils::Statistics::getRandomUniform01(true);
        const double u1 = Utils::Statistics::getRandomUniform01(true);
        const uint32_t ui = Utils::Statistics::getRandomUniformInt(1, 3, true);
        if (u0 < 0.5) {
            if (u1 < 0.5)
                em.submitLimitOrderEvent(Side::BUY, ui, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Side::SELL, ui, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Side::BUY, ui);
            else
                em.submitMarketOrderEvent(Side::SELL, ui);
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
                em.submitLimitOrderEvent(Side::BUY, ui, Utils::Statistics::getRandomUniform(e->getBestAskPrice() - 5.0, e->getBestAskPrice() - 1.0, true));
            else
                em.submitLimitOrderEvent(Side::SELL, ui, Utils::Statistics::getRandomUniform(e->getBestBidPrice() + 1.0, e->getBestBidPrice() + 5.0, true));
        } else {
            if (u1 < 0.5)
                em.submitMarketOrderEvent(Side::BUY, ui);
            else
                em.submitMarketOrderEvent(Side::SELL, ui);
        }
    }
    std::cout << *e << std::endl;
    std::cout << em << std::endl;
}

void testMatchingEngineOrderCancelModify() {
    // TODO
}

void testMatchingEngineZeroIntelligence() {
    // TODO
}
}
}

int main() {
    // Tests::MatchingEngine::testMatchingEngineSimpleBook();
    // Tests::MatchingEngine::testMatchingEngineOrderEventManager();
    Tests::MatchingEngine::testMatchingEngineRandomOrders();
    return 0;
}
