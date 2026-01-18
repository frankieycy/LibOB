#ifndef TEST_EXCHANGE_SIMULATOR_CPP
#define TEST_EXCHANGE_SIMULATOR_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderUtils.hpp"
#include "Exchange/MatchingEngine.hpp"
#include "Parser/LobsterDataParser.hpp"
#include "Simulator/VolumeProfile.hpp"
#include "Simulator/ZeroIntelligence.hpp"
#include "Analytics/MonitorOutputsAnalyzer.hpp"
#include "Tests/TestExchangeSimulator.hpp"

namespace Tests {
namespace ExchangeSimulator {
void testInitZeroIntelligenceSimulator() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 20, 2, 40), // linear interp from 2 @ 1 tick ($0.01) to 40 @ 20 ticks ($0.20)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(20, 40), // flat extrap at 40 beyond 20 ticks
        40);
    zi->setAnchorPrice(10.00);
    zi->setMinPriceTick(0.01);
    zi->initOrderBookBuilding(v0, v0); // linear book of $0.2 around the $10 anchor
    // print order book snapshot
    auto& config = e->getOrderBookDisplayConfig();
    config.setPrintAsciiOrderBook(true);
    config.setOrderBookLevels(30);
    std::cout << *e << std::endl;
}

void testZeroIntelligenceSimulatorRandomMarketOrders() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10 /* maxTimestamp */);
    // simulator-level config
    Simulator::ExchangeSimulatorConfig simConfig(zi->getConfig());
    simConfig.debugMode = true;
    simConfig.debugShowOrderBookPerEvent = true;
    // zero-intelligence config
    Simulator::ZeroIntelligenceConfig ziConfig(zi->getZIConfig());
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 10);
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 20, 2, 40), // linear interp from 2 @ 1 tick ($0.01) to 40 @ 20 ticks ($0.20)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(20, 40), // flat extrap at 40 beyond 20 ticks
        40);
    zi->setConfig(simConfig); // set the general configs first before altering the specifics with an exposed interface
    zi->setZIConfig(ziConfig);
    zi->setAnchorPrice(10.00);
    zi->setMinPriceTick(0.01);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $0.2 around the $10 anchor
    zi->simulate();
}

void testZeroIntelligenceSimulatorRandomMarketAndLimitOrders() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10 /* maxTimestamp */);
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.debugMode = true;
    simConfig.debugShowOrderBookPerEvent = true;
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 5);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::UniformOrderSizeSampler>(1, 5);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 10, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(100.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    // Lobster data export
    Parser::LobsterDataParser p;
    zi->getMatchingEngineMonitor()->exportToLobsterDataParser(p);
    const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
    std::cout << "Lobster messages:\n" << data.first << std::endl;
    std::cout << "Lobster snapshots:\n" << data.second << std::endl;
}

void testZeroIntelligenceSimulatorSimpleSantaFeModel() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::ExchangeSimulatorStopCondition stopCondition(10 /* maxTimestamp */);
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.debugMode = true;
    simConfig.debugShowOrderBookPerEvent = true;
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(1.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 10, zi->getMatchingEngineMonitor());
    ziConfig.cancelRateSampler = std::make_shared<Simulator::OrderEventRateSamplerProportionalTotalSizeFromOppositeBest>(0.02, 1, 10, zi->getMatchingEngineMonitor());
    ziConfig.cancelSideSampler = std::make_shared<Simulator::OrderSideSamplerProportionalTotalSizeFromOppositeBest>(1, 10, zi->getMatchingEngineMonitor());
    ziConfig.cancelSampler = std::make_shared<Simulator::OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest>(1, 1, 10, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(100.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    // Lobster data export
    Parser::LobsterDataParser p;
    zi->getMatchingEngineMonitor()->exportToLobsterDataParser(p);
    const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
    std::cout << "Lobster messages:\n" << data.first << std::endl;
    std::cout << "Lobster snapshots:\n" << data.second << std::endl;
}

void testZeroIntelligenceSimulatorSimpleSantaFeModelSpeedDiagnostics() {
    // use a speed profiler to analyze performance bottlenecks
    // cost centers: book monitor top-level snapshots (vector copies), Lobster data export (ostringstream dyanmic allocations)
    // 1. `make profiling` to compile in profiling mode with gperftools
    // 2. `CPUPROFILE=profile.out ./Exe/PROFILING/main` to run the executable with profiling enabled
    // 3. `pprof --top ./Exe/PROFILING/main profile.out` to see the text report of the profiling results
    // 4. `pprof --pdf ./Exe/PROFILING/main profile.out > profile.pdf` to generate a PDF visualization of the profiling results
    // 5. `pprof --http=:8080 Exe/PROFILING/main profile.out` to launch a web server for interactive visualization
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    Simulator::ExchangeSimulatorStopCondition stopCondition(1'000'000 /* maxTimestamp */); // as of Jan2026, 1 million events take about 8.5 seconds
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(10.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(30.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelRateSampler = std::make_shared<Simulator::OrderEventRateSamplerProportionalTotalSizeFromOppositeBest>(0.2, 1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSideSampler = std::make_shared<Simulator::OrderSideSamplerProportionalTotalSizeFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSampler = std::make_shared<Simulator::OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest>(1, 1, 30, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(1000.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    // Lobster data export
    Parser::LobsterDataParser p;
    zi->getMatchingEngineMonitor()->exportToLobsterDataParser(p);
    const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
}

void testZeroIntelligenceSimulatorSimpleSantaFeModelAsymptoticStats() {
    std::shared_ptr<Exchange::MatchingEngineFIFO> e = std::make_shared<Exchange::MatchingEngineFIFO>();
    std::shared_ptr<Simulator::ZeroIntelligenceSimulator> zi = std::make_shared<Simulator::ZeroIntelligenceSimulator>(e);
    std::shared_ptr<Analytics::MatchingEngineMonitorOutputsAnalyzer> a = std::make_shared<Analytics::MatchingEngineMonitorOutputsAnalyzer>(zi->getMatchingEngineMonitor());
    Simulator::ExchangeSimulatorStopCondition stopCondition(1'000'000 /* maxTimestamp */);
    // simulator-level config
    auto& simConfig = zi->getConfig();
    simConfig.resetMatchingEngineMonitorPreSimulation = true;
    // zero-intelligence config
    auto& ziConfig = zi->getZIConfig();
    ziConfig.marketOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(10.0);
    ziConfig.marketSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitOrderRateSampler = std::make_shared<Simulator::ConstantOrderEventRateSampler>(30.0);
    ziConfig.limitSizeSampler = std::make_shared<Simulator::ConstantOrderSizeSampler>(1);
    ziConfig.limitPriceSampler = std::make_shared<Simulator::OrderPricePlacementSamplerUniformFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelRateSampler = std::make_shared<Simulator::OrderEventRateSamplerProportionalTotalSizeFromOppositeBest>(0.2, 1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSideSampler = std::make_shared<Simulator::OrderSideSamplerProportionalTotalSizeFromOppositeBest>(1, 30, zi->getMatchingEngineMonitor());
    ziConfig.cancelSampler = std::make_shared<Simulator::OrderCancellationSamplerConstantSizeUniformPriceFromOppositeBest>(1, 1, 30, zi->getMatchingEngineMonitor());
    // initial volume profile
    Simulator::VolumeProfile v0(
        std::make_unique<Simulator::LinearVolumeInterpolator>(1, 10, 1, 5), // linear interp from 1 @ 1 tick ($1) to 5 @ 10 ticks ($10)
        std::make_unique<Simulator::FlatVolumeExtrapolator>(10, 5), // flat extrap at 5 beyond 10 ticks
        10);
    zi->setAnchorPrice(1000.0);
    zi->setMinPriceTick(1.0);
    zi->setStopCondition(stopCondition);
    zi->initOrderBookBuilding(v0, v0); // linear book of $10 around the $100 anchor
    zi->simulate();
    auto& statsConfig = a->getStatsConfig();
    statsConfig.orderDepthProfileConfig.minPriceTick = zi->getMinPriceTick();
    statsConfig.orderDepthProfileConfig.maxTicks = zi->getMonitoredOrderBookNumLevels();
    statsConfig.orderLifetimeStatsConfig.minPriceTick = zi->getMinPriceTick();
    statsConfig.orderLifetimeStatsConfig.maxTicks = zi->getMonitoredOrderBookNumLevels();
    statsConfig.spreadStatsConfig.maxSpread = 100.0 * zi->getMinPriceTick();
    statsConfig.spreadStatsConfig.numBins = 100; // 100 bins over $0 to $100
    a->updateStatsConfig();
    a->populateOrderBookTraces();
    a->runAnalytics();
    // Lobster data export
    // Parser::LobsterDataParser p;
    // zi->getMatchingEngineMonitor()->exportToLobsterDataParser(p);
    // const auto data = p.getOrderBookMessagesAndSnapshotsAsCsv(5, true);
}
}
}

#endif
