#ifndef TEST_MATCHING_ENGINE_HPP
#define TEST_MATCHING_ENGINE_HPP

namespace Tests {
namespace MatchingEngine {
void testPrintOrderBookASCII();
void testMatchingEngineSimpleBook();
void testMatchingEngineOrderEventManager();
void testMatchingEngineRandomOrders();
void testMatchingEngineOrderCancelModify();
void testMatchingEngineOrderCancelReplace();
void testMatchingEngineRandomOrdersSpeedTest();
void testMatchingEngineRandomOrdersStressTest();
void testMatchingEngineSpeedProfiling();
void testMatchingEngineConstructFromEventsStream();
void testMatchingEngineGetAsJson();
void testMatchingEngineITCHMessage();
void testMatchingEngineMonitor();
void testMatchingEngineZeroIntelligence();
}
}

#endif
