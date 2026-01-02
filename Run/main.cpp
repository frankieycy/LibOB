#include "Utils/Utils.hpp"
#include "Tests/TestMatchingEngine.hpp"
#include "Tests/TestExchangeSimulator.hpp"

namespace {
inline void printProgramOpening() { Utils::IO::printLibOBBanner(std::cout); }
}

int main() {
    printProgramOpening();
    // Tests::MatchingEngine::testPrintOrderBookASCII();
    // Tests::MatchingEngine::testMatchingEngineSimpleBook();
    // Tests::MatchingEngine::testMatchingEngineOrderEventManager();
    // Tests::MatchingEngine::testMatchingEngineRandomOrders();
    // Tests::MatchingEngine::testMatchingEngineOrderCancelModify();
    // Tests::MatchingEngine::testMatchingEngineOrderCancelReplace();
    // Tests::MatchingEngine::testMatchingEngineRandomOrdersSpeedTest();
    // Tests::MatchingEngine::testMatchingEngineRandomOrdersStressTest();
    // Tests::MatchingEngine::testMatchingEngineSpeedProfiling();
    // Tests::MatchingEngine::testMatchingEngineConstructFromEventsStream();
    // Tests::MatchingEngine::testMatchingEngineGetAsJson();
    // Tests::MatchingEngine::testMatchingEngineMonitor();
    // Tests::MatchingEngine::testMatchingEngineMonitorLobsterOutput();
    // Tests::ExchangeSimulator::testInitZeroIntelligenceSimulator();
    // Tests::ExchangeSimulator::testZeroIntelligenceSimulatorRandomMarketOrders();
    // Tests::ExchangeSimulator::testZeroIntelligenceSimulatorRandomMarketAndLimitOrders();
    // Tests::ExchangeSimulator::testZeroIntelligenceSimulatorSimpleSantaFeModel();
    // Tests::ExchangeSimulator::testZeroIntelligenceSimulatorSimpleSantaFeModelSpeedDiagnostics();
    Tests::ExchangeSimulator::testZeroIntelligenceSimulatorSimpleSantaFeModelAsymptoticStats();
    return 0;
}
