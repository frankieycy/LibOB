#ifndef ZERO_INTELLIGENCE_HPP
#define ZERO_INTELLIGENCE_HPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;

struct ZeroIntelligenceConfig {
    // TODO
};

class ZeroIntelligenceSimulator : public ExchangeSimulatorBase {
public:
    ZeroIntelligenceSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~ZeroIntelligenceSimulator() = default;
    virtual void init() override;
    const ZeroIntelligenceConfig& getZIConfig() const { return myZIConfig; }
    void setZIConfig(const ZeroIntelligenceConfig& config) { myZIConfig = config; }
private:
    std::shared_ptr<IEventScheduler> makeEventScheduler() const override;
    std::shared_ptr<OrderEventBase> generateNextOrderEvent() const;
    ZeroIntelligenceConfig myZIConfig = ZeroIntelligenceConfig();
};
}

#endif
