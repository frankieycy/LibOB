#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Market/OrderEventManager.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Analytics/MatchingEngineMonitor.hpp"
#include "Parser/LobsterDataParser.hpp"

const std::string TEST_NAME = "MatchingEngineMonitorLobsterOutput";

int main() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    Market::OrderEventManagerBase em{e};
    Analytics::MatchingEngineMonitor m{e};
    Parser::LobsterDataParser p;
    em.setLoggerLogFile(Utils::RegressionTests::getBaselineFileName(TEST_NAME), false, false);
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
    // export data in lobster format
    m.exportToLobsterDataParser(p);
    const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
    *em.getLogger() << "Lobster messages:\n" << data.first;
    *em.getLogger() << "Lobster snapshots:\n" << data.second;
    return 0;
}
