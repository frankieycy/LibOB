#include "Utils.hpp"
#include "OrderUtils.hpp"
#include "Order.hpp"
#include "MatchingEngine.hpp"
using namespace Market;
using namespace Exchange;

int main() {
    MatchingEngineFIFO e;
    const std::shared_ptr<LimitOrder> b1 = std::make_shared<LimitOrder>(1, 0, Side::BUY, 10, 99.0);
    const std::shared_ptr<LimitOrder> b2 = std::make_shared<LimitOrder>(2, 0, Side::BUY, 15, 98.0);
    const std::shared_ptr<LimitOrder> b3 = std::make_shared<LimitOrder>(3, 0, Side::BUY, 20, 97.0);
    const std::shared_ptr<LimitOrder> a1 = std::make_shared<LimitOrder>(4, 0, Side::SELL, 10, 101.0);
    const std::shared_ptr<LimitOrder> a2 = std::make_shared<LimitOrder>(5, 0, Side::SELL, 15, 102.0);
    const std::shared_ptr<LimitOrder> a3 = std::make_shared<LimitOrder>(6, 0, Side::SELL, 20, 103.0);
    const std::shared_ptr<MarketOrder> m1 = std::make_shared<MarketOrder>(7, 1, Side::BUY, 5);
    const std::shared_ptr<MarketOrder> m2 = std::make_shared<MarketOrder>(8, 1, Side::SELL, 15);
    const std::vector<const std::shared_ptr<LimitOrder>> limitQueue{b1, b2, b3, a1, a2, a3};
    const std::vector<const std::shared_ptr<MarketOrder>> marketQueue{m1, m2};
    for (const auto& o : limitQueue) {
        std::cout << "Add to limit order book: " << *o << std::endl;
        e.addToLimitOrderBook(o);
    }
    for (const auto& o : marketQueue) {
        std::cout << "Execute market order: " << *o << std::endl;
        e.executeMarketOrder(o);
    }
    return 0;
}
