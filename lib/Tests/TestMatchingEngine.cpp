#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Tests/TestMatchingEngine.hpp"

namespace Tests {
namespace MatchingEngine {
void testPrintOrderBookASCII() {
    std::vector<Exchange::OrderLevel> bids = {
        {98.00, 3}, {97.57, 1}, {97.00, 7}, {96.00, 8}, {95.00, 9}, {94.00, 10}
    };
    std::vector<Exchange::OrderLevel> asks = {
        {100.39, 1}, {101.46, 3}, {102.07, 2}, {107.00, 10}, {108.00, 10}, {110.00, 10}
    };
    std::cout << getOrderBookASCII(bids, asks);
}

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
    Utils::IO::printDebugBanner(std::cout);
    std::cout << *e << std::endl;
    Utils::IO::printDebugBanner(std::cout);
    std::cout << em << std::endl;
}

void testMatchingEngineOrderCancelModify() {
    // test order cancellation and modification
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>(true);
    Market::OrderEventManagerBase em{e};
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
}

void testMatchingEngineRandomOrdersSpeedTest() {
    // run market dynamics by bulk submission of orders to test the speed of various matching engine operations
    // e.g. order submission, cancellation, modification, and execution
    // sample outputs ==================================================================================================
    // Timing stats (ns) per limit order submit: {"size":1000000,"mean":359.241,"variance":3.80675e+08,"stddev":19510.9}
    // Timing stats (ns) per limit order cancel: {"size":200000,"mean":1067.45,"variance":1.00545e+07,"stddev":3170.88}
    // Timing stats (ns) per limit order modify price: {"size":200000,"mean":421.857,"variance":196068,"stddev":442.796}
    // Timing stats (ns) per limit order modify quantity: {"size":200000,"mean":856.257,"variance":241391,"stddev":491.316}
    // Timing stats (ns) per market order submit: {"size":200000,"mean":2417.06,"variance":1.90288e+09,"stddev":43622}
    const int numOrders = 1000000; // 1 million orders
    const std::vector<double> p{0, 1, 3, 5, 7, 9, 6, 3, 2, 1, 1, 1, 1}; // relative probabilities for order book levels
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    em.reserve(numOrders);
    std::vector<long long> timesPerLimitOrderSubmit;
    std::vector<long long> timesPerLimitOrderCancel;
    std::vector<long long> timesPerLimitOrderModifyPrice;
    std::vector<long long> timesPerLimitOrderModifyQuantity;
    std::vector<long long> timesPerMarketOrderSubmit;
    // order submission
    for (int i = 0; i < numOrders; ++i) {
        const double u = Utils::Statistics::getRandomUniform01(true);
        const int qty = Utils::Statistics::getRandomUniformInt(1, 3, true);
        const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
        if (u < 0.5)
            timesPerLimitOrderSubmit.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, qty, j]() { em.submitLimitOrderEvent(Market::Side::BUY, qty, 100.0 - j); }));
        else
            timesPerLimitOrderSubmit.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, qty, j]() { em.submitLimitOrderEvent(Market::Side::SELL, qty, 100.0 + j); }));
    }
    std::cout << "Timing stats (ns) per limit order submit: " << Utils::Statistics::getVectorStats(timesPerLimitOrderSubmit) << std::endl;
    // order cancellation
    const auto& activeOrders = em.getActiveLimitOrders();
    std::vector<uint64_t> activeOrderIds;
    activeOrderIds.reserve(activeOrders.size());
    for (const auto& orderPair : activeOrders)
        activeOrderIds.push_back(orderPair.first);
    for (int i = 0; i < numOrders / 5; ++i) {
        // const auto& it = Utils::Statistics::drawRandomIterator(activeOrders, true); // advancing unordered_map iterator works in O(n) - slow!
        // timesPerLimitOrderCancel.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, it]() { em.cancelOrder(it->first); }));
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        timesPerLimitOrderCancel.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, orderId]() { em.cancelOrder(orderId); }));
        activeOrderIds[randomIndex] = activeOrderIds.back(); // swap and pop
        activeOrderIds.pop_back();
    }
    std::cout << "Timing stats (ns) per limit order cancel: " << Utils::Statistics::getVectorStats(timesPerLimitOrderCancel) << std::endl;
    // order price modification
    for (int i = 0; i < numOrders / 5; ++i) {
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        const auto& order = activeOrders.at(orderId);
        const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
        const double modifiedPrice = order->isBuy() ? 100.0 - j : 100.0 + j;
        timesPerLimitOrderModifyPrice.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, orderId, modifiedPrice]() { em.modifyOrderPrice(orderId, modifiedPrice); }));
    }
    std::cout << "Timing stats (ns) per limit order modify price: " << Utils::Statistics::getVectorStats(timesPerLimitOrderModifyPrice) << std::endl;
    // order quantity modification
    for (int i = 0; i < numOrders / 5; ++i) {
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        const double modifiedQuantity = Utils::Statistics::getRandomUniformInt(1, 3, true);
        timesPerLimitOrderModifyQuantity.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, orderId, modifiedQuantity]() { em.modifyOrderQuantity(orderId, modifiedQuantity); }));
    }
    std::cout << "Timing stats (ns) per limit order modify quantity: " << Utils::Statistics::getVectorStats(timesPerLimitOrderModifyQuantity) << std::endl;
    // market order submission
    for (int i = 0; i < numOrders / 5; ++i) {
        const double u = Utils::Statistics::getRandomUniform01(true);
        const int qty = Utils::Statistics::getRandomUniformInt(1, 10, true);
        if (u < 0.5)
            timesPerMarketOrderSubmit.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, qty]() { em.submitMarketOrderEvent(Market::Side::BUY, qty); }));
        else
            timesPerMarketOrderSubmit.push_back(Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, qty]() { em.submitMarketOrderEvent(Market::Side::SELL, qty); }));
    }
    std::cout << "Timing stats (ns) per market order submit: " << Utils::Statistics::getVectorStats(timesPerMarketOrderSubmit) << std::endl;
    // final order book state
    Utils::IO::printDebugBanner(std::cout);
    auto& config = e->getOrderBookDisplayConfig();
    config.setPrintAsciiOrderBook(true);
    config.setOrderBookLevels(20);
    std::cout << *e << std::endl;
}

void testMatchingEngineRandomOrdersStressTest() {
    // run market dynamics by bulk submission of orders to stress test the matching engine
    // sample outputs =======================================================
    // Timing taken for bulk order submission: 10.7403 secs / 10000000 orders
    // Timing taken for bulk order cancel/modify: 53.3957 secs / 10000000 orders
    const int numOrders = 10000000; // 10 million orders
    const std::vector<double> p{0, 1, 3, 5, 7, 9, 6, 3, 2, 1, 1, 1, 1}; // relative probabilities for order book levels
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    em.reserve(numOrders);
    // stress test mixed limit and market order submission
    const auto timeOrderSubmit = Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, &p]() {
        for (int i = 0; i < numOrders; ++i) {
            const double u0 = Utils::Statistics::getRandomUniform01(true);
            const double u1 = Utils::Statistics::getRandomUniform01(true);
            const int qty0 = Utils::Statistics::getRandomUniformInt(1, 3, true);
            const int qty1 = Utils::Statistics::getRandomUniformInt(1, 5, true);
            const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
            if (u0 < 0.7) {
                if (u1 < 0.5)
                    em.submitLimitOrderEvent(Market::Side::BUY, qty0, 100.0 - j);
                else
                    em.submitLimitOrderEvent(Market::Side::SELL, qty0, 100.0 + j);
            } else {
                if (u1 < 0.5)
                    em.submitMarketOrderEvent(Market::Side::BUY, qty1);
                else
                    em.submitMarketOrderEvent(Market::Side::SELL, qty1);
            }
        }
    });
    std::cout << "Timing taken for bulk order submission: " << timeOrderSubmit / 1e9 << " secs / " << numOrders << " orders" << std::endl;
    // stress test mixed order cancellation and modification
    const auto timeOrderCancelModify = Utils::Counter::timeOperation<std::chrono::nanoseconds>([&em, &p]() {
        const auto& activeOrders = em.getActiveLimitOrders();
        std::vector<uint64_t> activeOrderIds;
        activeOrderIds.reserve(activeOrders.size());
        for (const auto& orderPair : activeOrders)
            activeOrderIds.push_back(orderPair.first);
        for (int i = 0; i < numOrders; ++i) {
            const double u = Utils::Statistics::getRandomUniform01(true);
            const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
            const uint64_t orderId = activeOrderIds[randomIndex];
            if (u < 0.2) { // cancel
                em.cancelOrder(orderId);
                activeOrderIds[randomIndex] = activeOrderIds.back(); // swap and pop
                activeOrderIds.pop_back();
            } else if (u < 0.6) { // modify price
                const auto& order = activeOrders.at(orderId);
                const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
                const double modifiedPrice = order->isBuy() ? 100.0 - j : 100.0 + j;
                em.modifyOrderPrice(orderId, modifiedPrice);
            } else { // modify quantity
                const double modifiedQuantity = Utils::Statistics::getRandomUniformInt(1, 3, true);
                em.modifyOrderQuantity(orderId, modifiedQuantity);
            }
        }
    });
    std::cout << "Timing taken for bulk order cancel/modify: " << timeOrderCancelModify / 1e9 << " secs / " << numOrders << " orders" << std::endl;
    // final order book state
    Utils::IO::printDebugBanner(std::cout);
    auto& config = e->getOrderBookDisplayConfig();
    config.setPrintAsciiOrderBook(true);
    config.setOrderBookLevels(20);
    std::cout << *e << std::endl;
}

void testMatchingEngineSpeedProfiling() {
    // detailed speed profiling of matching engine operations - set numerous timers inside MatchingEngine::process(event)
    const int numOrders = 250;
    const std::vector<double> p{0, 1, 3, 5, 7, 9, 6, 3, 2, 1, 1, 1, 1}; // relative probabilities for order book levels
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    em.reserve(numOrders);
    // order submission
    std::cout << "Submitting " << numOrders << " random limit orders..." << std::endl;
    for (int i = 0; i < numOrders; ++i) {
        const double u = Utils::Statistics::getRandomUniform01(true);
        const int qty = Utils::Statistics::getRandomUniformInt(1, 3, true);
        const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
        if (u < 0.5)
            em.submitLimitOrderEvent(Market::Side::BUY, qty, 100.0 - j);
        else
            em.submitLimitOrderEvent(Market::Side::SELL, qty, 100.0 + j);
    }
    // order cancellation
    // sample outputs ===========================================
    // Time (ns) myLimitOrderLookup.find(event->getOrderId()): 42
    // Time (ns) order->executeOrderEvent(*event): 41
    // Time (ns) !order->isAlive() pre-processing: 2834
    // Time (ns) !order->isAlive() book update: 2708
    // Time (ns) !order->isAlive() order report: 3000
    std::cout << "\n\nCancelling " << numOrders / 5 << " random limit orders..." << std::endl;
    const auto& activeOrders = em.getActiveLimitOrders();
    std::vector<uint64_t> activeOrderIds;
    activeOrderIds.reserve(activeOrders.size());
    for (const auto& orderPair : activeOrders)
        activeOrderIds.push_back(orderPair.first);
    for (int i = 0; i < numOrders / 5; ++i) {
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        em.cancelOrder(orderId);
        activeOrderIds[randomIndex] = activeOrderIds.back(); // swap and pop
        activeOrderIds.pop_back();
    }
    // order price modification
    // sample outputs ===========================================
    // Time (ns) myLimitOrderLookup.find(event->getOrderId()): 41
    // Time (ns) order->executeOrderEvent(*event): 42
    // Time (ns) order->isAlive() pre-processing: 2208
    // Time (ns) order->isAlive() book update: 2458
    // Time (ns) order->isAlive() order report: 2417
    std::cout << "\n\nModifying price of " << numOrders / 5 << " random limit orders..." << std::endl;
    for (int i = 0; i < numOrders / 5; ++i) {
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        const auto& order = activeOrders.at(orderId);
        const size_t j = Utils::Statistics::drawIndexWithRelativeProbabilities(p, true);
        const double modifiedPrice = order->isBuy() ? 100.0 - j : 100.0 + j;
        em.modifyOrderPrice(orderId, modifiedPrice);
    }
    // order quantity modification
    // sample outputs ===========================================
    // Time (ns) myLimitOrderLookup.find(event->getOrderId()): 42
    // Time (ns) order->executeOrderEvent(*event): 41
    // Time (ns) order->isAlive() pre-processing: 1792
    // Time (ns) order->isAlive() book update: 1958
    // Time (ns) order->isAlive() order report: 2000
    std::cout << "\n\nModifying quantity of " << numOrders / 5 << " random limit orders..." << std::endl;
    for (int i = 0; i < numOrders / 5; ++i) {
        const size_t randomIndex = Utils::Statistics::getRandomUniformInt(static_cast<size_t>(0), activeOrderIds.size() - 1, true);
        const uint64_t orderId = activeOrderIds[randomIndex];
        const double modifiedQuantity = Utils::Statistics::getRandomUniformInt(1, 3, true);
        em.modifyOrderQuantity(orderId, modifiedQuantity);
    }
    // final order book state
    Utils::IO::printDebugBanner(std::cout);
    auto& config = e->getOrderBookDisplayConfig();
    config.setPrintAsciiOrderBook(true);
    config.setOrderBookLevels(20);
    std::cout << *e << std::endl;
}

void testMatchingEngineConstructFromEventsStream() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    // initial book state
    for (int i = 0; i < 20; ++i) {
        em.submitLimitOrderEvent(Market::Side::BUY, std::min(5 + i, 10), 99.0 - i);
        em.submitLimitOrderEvent(Market::Side::SELL, std::min(5 + i, 10), 101.0 + i);
    }
    // random limit and market orders
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
    std::cout << *e << std::endl;
    // construct new matching engine from order event logs
    Utils::IO::printDebugBanner(std::cout);
    std::shared_ptr<Exchange::MatchingEngineFIFO> e1 = std::make_shared<Exchange::MatchingEngineFIFO>();
    e1->setLogger(em.getLogger());
    e1->build(e->getOrderEventLog());
    std::cout << *e1 << std::endl;
    // construct new matching engine from order report logs
    Utils::IO::printDebugBanner(std::cout);
    std::shared_ptr<Exchange::MatchingEngineFIFO> e2 = std::make_shared<Exchange::MatchingEngineFIFO>();
    e2->setLogger(em.getLogger());
    e2->build(e->getOrderProcessingReportLog());
    std::cout << *e2 << std::endl;
}

void testMatchingEngineGetAsJson() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    // initial book state
    for (int i = 0; i < 20; ++i) {
        em.submitLimitOrderEvent(Market::Side::BUY, std::min(5 + i, 10), 99.0 - i);
        em.submitLimitOrderEvent(Market::Side::SELL, std::min(5 + i, 10), 101.0 + i);
    }
    // get matching engine state as Json
    std::cout << "Matching engine Json:\n" << e->getAsJson() << std::endl;
}

void testMatchingEngineZeroIntelligence() {
    // TODO
}
}
}
