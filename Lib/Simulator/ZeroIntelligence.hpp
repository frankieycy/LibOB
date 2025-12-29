#ifndef ZERO_INTELLIGENCE_HPP
#define ZERO_INTELLIGENCE_HPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulator.hpp"
#include "Simulator/ZeroIntelligenceUtils.hpp"

namespace Simulator {
using namespace Utils;

struct ZeroIntelligenceConfig {
    // samplers for market order submit event
    std::shared_ptr<IOrderEventRateSampler> marketOrderRateSampler;
    std::shared_ptr<IOrderSideSampler> marketSideSampler;
    std::shared_ptr<IOrderSizeSampler> marketSizeSampler;
    // samplers for limit order submit event
    std::shared_ptr<IOrderEventRateSampler> limitOrderRateSampler;
    std::shared_ptr<IOrderSideSampler> limitSideSampler;
    std::shared_ptr<IOrderSizeSampler> limitSizeSampler;
    std::shared_ptr<IOrderPricePlacementSampler> limitPriceSampler;
    // samplers for limit order cancel event
    std::shared_ptr<IOrderEventRateSampler> cancelRateSampler;
    std::shared_ptr<IOrderSideSampler> cancelSideSampler;
    std::shared_ptr<IOrderCancellationSampler> cancelSampler;
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
