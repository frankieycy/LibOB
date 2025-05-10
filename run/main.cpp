#include "Utils.hpp"
#include "OrderUtils.hpp"
#include "Order.hpp"
#include "MatchingEngine.hpp"
using namespace Market;
using namespace Exchange;

namespace Tests {
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
}

int main() {
    Tests::testMatchingEngineSimpleBook();
    return 0;
}
