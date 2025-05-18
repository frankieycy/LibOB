#include "Utils.hpp"
#include "OrderUtils.hpp"
#include "Order.hpp"
#include "MatchingEngine.hpp"
#include "OrderEventManager.hpp"

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
    MatchingEngineFIFO e{true};
    OrderEventManagerBase em{std::make_shared<MatchingEngineFIFO>(e)};
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
    // TODO
}

void testMatchingEngineZeroIntelligence() {
    // TODO
}
}
}

int main() {
    // Tests::MatchingEngine::testMatchingEngineSimpleBook();
    Tests::MatchingEngine::testMatchingEngineOrderEventManager();
    return 0;
}
