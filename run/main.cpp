#include "Utils/Utils.hpp"
#include "Tests/TestMatchingEngine.hpp"

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
    // Tests::MatchingEngine::testMatchingEngineRandomOrdersSpeedTest();
    // Tests::MatchingEngine::testMatchingEngineRandomOrdersStressTest();
    // Tests::MatchingEngine::testMatchingEngineSpeedProfiling();
    // Tests::MatchingEngine::testMatchingEngineConstructFromEventsStream();
    Tests::MatchingEngine::testMatchingEngineGetAsJson();
    return 0;
}
