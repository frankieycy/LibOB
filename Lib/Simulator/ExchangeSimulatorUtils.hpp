#ifndef EXCHANGE_SIMULATOR_UTILS_HPP
#define EXCHANGE_SIMULATOR_UTILS_HPP
#include "Utils/Utils.hpp"

namespace Simulator {
using namespace Utils;

enum class ExchangeSimulatorState { UNINITIALIZED, READY, RUNNING, FINISHED };
enum class ExchangeSimulatorType { ZERO_INTELLIGENCE, MINIMAL_INTELLIGENCE, NULL_EXCHANGE_SIMULATOR_TYPE };

struct OrderBookGridDefinition {
    double anchorPrice = Consts::NAN_DOUBLE; // defines a small- or large-tick stock
    double minPriceTick = 0.01; // passes down to order event manager
    uint32_t minLotSize = 1;
    uint32_t numGrids = 10000; // total number of price grids on one side of the book
};

struct ExchangeSimulatorConfig {
    bool debugMode = false; // passes down to matching engine
    size_t monitoredLevels = 100; // passes down to matching engine monitor
    uint64_t randomSeed = 42;
    OrderBookGridDefinition grid;
};
}

#endif
