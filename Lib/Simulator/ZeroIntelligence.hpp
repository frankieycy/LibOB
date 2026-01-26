#ifndef ZERO_INTELLIGENCE_HPP
#define ZERO_INTELLIGENCE_HPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulator.hpp"
#include "Simulator/ZeroIntelligenceUtils.hpp"

namespace Simulator {
struct ZeroIntelligenceConfig {
    // samplers for market order submit event
    std::shared_ptr<IOrderEventRateSampler> marketOrderRateSampler = std::make_shared<ConstantOrderEventRateSampler>(0.0);
    std::shared_ptr<IOrderSideSampler> marketSideSampler = std::make_shared<UniformOrderSideSampler>();
    std::shared_ptr<IOrderSizeSampler> marketSizeSampler = std::make_shared<NullOrderSizeSampler>();
    // samplers for limit order submit event
    std::shared_ptr<IOrderEventRateSampler> limitOrderRateSampler = std::make_shared<ConstantOrderEventRateSampler>(0.0);
    std::shared_ptr<IOrderSideSampler> limitSideSampler = std::make_shared<UniformOrderSideSampler>();
    std::shared_ptr<IOrderSizeSampler> limitSizeSampler = std::make_shared<NullOrderSizeSampler>();
    std::shared_ptr<IOrderPricePlacementSampler> limitPriceSampler = std::make_shared<NullOrderPricePlacementSampler>();
    // samplers for limit order cancel event
    std::shared_ptr<IOrderEventRateSampler> cancelRateSampler = std::make_shared<ConstantOrderEventRateSampler>(0.0);
    std::shared_ptr<IOrderSideSampler> cancelSideSampler = std::make_shared<UniformOrderSideSampler>();
    std::shared_ptr<IOrderCancellationSampler> cancelSampler = std::make_shared<NullOrderCancellationSampler>();
};

class ZeroIntelligenceSimulator : public ExchangeSimulatorBase {
public:
    ZeroIntelligenceSimulator() = delete;
    ZeroIntelligenceSimulator(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~ZeroIntelligenceSimulator() = default;
    virtual void init() override;
    ZeroIntelligenceConfig& getZIConfig() { return myZIConfig; }
    const ZeroIntelligenceConfig& getZIConfig() const { return myZIConfig; }
    void setZIConfig(const ZeroIntelligenceConfig& config) { myZIConfig = config; }
private:
    std::shared_ptr<IEventScheduler> makeEventScheduler() const override;
    std::shared_ptr<OrderEventBase> generateNextOrderEvent() const;
    ZeroIntelligenceConfig myZIConfig = ZeroIntelligenceConfig();
};
}

#endif
