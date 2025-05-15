#ifndef ORDER_EVENT_MANAGER_HPP
#define ORDER_EVENT_MANAGER_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "OrderEvent.hpp"
#include "MatchingEngine.hpp"

namespace Market {
class OrderEventManagerBase {
public:
    OrderEventManagerBase() = default;
    // OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    // virtual const std::shared_ptr<OrderSubmitEvent> submitLimitOrder(const Side side, const uint32_t quantity, const double price);
    // virtual const std::shared_ptr<OrderCancelEvent> cancelOrder(const uint64_t orderId);
    // virtual const std::shared_ptr<OrderModifyPriceEvent> modifyOrderPrice(const uint64_t orderId, const double modifiedPrice);
    // virtual const std::shared_ptr<OrderModifyQuantityEvent> modifyOrderQuantity(const uint64_t orderId, const double modifiedQuantity);
private:
    Utils::Counter::IdHandlerBase myOrderIdHandler = Utils::Counter::IdHandlerBase();
    Utils::Counter::IdHandlerBase myEventIdHandler = Utils::Counter::IdHandlerBase();
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> myWorldClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    std::unordered_map<uint64_t, std::shared_ptr<Market::OrderEventBase>> myActiveOrders;
    bool mySyncClockWithEngine = true;
};
}

#endif
