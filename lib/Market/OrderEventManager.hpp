#ifndef ORDER_EVENT_MANAGER_HPP
#define ORDER_EVENT_MANAGER_HPP
#include "Utils/Utils.hpp"
#include "Market/Order.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Market {
using LimitOrderIndex = std::unordered_map<uint64_t, std::shared_ptr<Market::LimitOrder>>;
using MarketOrderIndex = std::unordered_map<uint64_t, std::shared_ptr<Market::MarketOrder>>;

class OrderEventManagerBase {
public:
    OrderEventManagerBase() = default;
    OrderEventManagerBase(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine);
    virtual ~OrderEventManagerBase() = default;
    const Utils::Counter::IdHandlerBase& getOrderIdHandler() const { return myOrderIdHandler; }
    const Utils::Counter::IdHandlerBase& getEventIdHandler() const { return myEventIdHandler; }
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> getWorldClock() const { return myWorldClock; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    std::shared_ptr<const Exchange::IMatchingEngine> getMatchingEngine() const { return myMatchingEngine; }
    const LimitOrderIndex& getActiveLimitOrders() const { return myActiveLimitOrders; }
    const MarketOrderIndex& getQueuedMarketOrders() const { return myQueuedMarketOrders; }
    double getMinimumPriceTick() const { return myMinimumPriceTick; }
    long long getMillisecondsToPauseBeforeEventSubmit() const { return myMillisecondsToPauseBeforeEventSubmit; }
    bool isTimeEngineOrderEventsProcessing() const { return myTimeEngineOrderEventsProcessing; }
    bool isSyncClockWithEngine() const { return mySyncClockWithEngine; }
    bool isDebugMode() const { return myDebugMode; }
    bool isPrintOrderBookPerOrderSubmit() const { return myPrintOrderBookPerOrderSubmit; }
    void setOrderIdHandler(const Utils::Counter::IdHandlerBase& orderIdHandler) { myOrderIdHandler = orderIdHandler; }
    void setEventIdHandler(const Utils::Counter::IdHandlerBase& eventIdHandler) { myEventIdHandler = eventIdHandler; }
    void setWorldClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& worldClock) { myWorldClock = worldClock; }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    void setMatchingEngine(const std::shared_ptr<Exchange::IMatchingEngine>& matchingEngine) { myMatchingEngine = matchingEngine; }
    void setMinimumPriceTick(const double minimumPriceTick) { myMinimumPriceTick = minimumPriceTick; }
    void setMillisecondsToPauseBeforeEventSubmit(const long long millisecondsToPauseBeforeEventSubmit) { myMillisecondsToPauseBeforeEventSubmit = millisecondsToPauseBeforeEventSubmit; }
    void setTimeEngineOrderEventsProcessing(const bool timeEngineOrderEventsProcessing) { myTimeEngineOrderEventsProcessing = timeEngineOrderEventsProcessing; }
    void setSyncClockWithEngine(const bool syncClockWithEngine) { mySyncClockWithEngine = syncClockWithEngine; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }
    void setPrintOrderBookPerOrderSubmit(const bool printOrderBookPerOrderSubmit) { myPrintOrderBookPerOrderSubmit = printOrderBookPerOrderSubmit; }
    void setLoggerLogFile(const std::string& logFileName, const bool showLogTimestamp = true);
    uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return myWorldClock->tick(elapsedTimeUnit); }
    std::shared_ptr<const OrderSubmitEvent> submitLimitOrderEvent(const Side side, const uint32_t quantity, const double price);
    std::shared_ptr<const OrderSubmitEvent> submitMarketOrderEvent(const Side side, const uint32_t quantity);
    std::shared_ptr<const OrderCancelEvent> cancelOrder(const uint64_t orderId);
    std::shared_ptr<const OrderModifyPriceEvent> modifyOrderPrice(const uint64_t orderId, const double modifiedPrice);
    std::shared_ptr<const OrderModifyQuantityEvent> modifyOrderQuantity(const uint64_t orderId, const double modifiedQuantity);
    // communicates with matching engine to keep ActiveOrders in sync
    virtual void onOrderProcessingReport(const Exchange::OrderExecutionReport& report);
    virtual void onOrderProcessingReport(const Exchange::LimitOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::MarketOrderSubmitReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderCancelReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyPriceReport& report);
    virtual void onOrderProcessingReport(const Exchange::OrderModifyQuantityReport& report);
    virtual std::ostream& stateSnapshot(std::ostream& out) const;
    virtual void reserve(const size_t numOrdersEstimate);
private:
    void submitOrderEventToMatchingEngine(const std::shared_ptr<OrderEventBase>& event);
    virtual std::shared_ptr<OrderSubmitEvent> createLimitOrderSubmitEvent(const Side side, const uint32_t quantity, const double price);
    virtual std::shared_ptr<OrderSubmitEvent> createMarketOrderSubmitEvent(const Side side, const uint32_t quantity);
    virtual std::shared_ptr<OrderCancelEvent> createOrderCancelEvent(const uint64_t orderId);
    virtual std::shared_ptr<OrderModifyPriceEvent> createOrderModifyPriceEvent(const uint64_t orderId, const double modifiedPrice);
    virtual std::shared_ptr<OrderModifyQuantityEvent> createOrderModifyQuantityEvent(const uint64_t orderId, const double modifiedQuantity);
    virtual std::shared_ptr<OrderBase> fetchOrder(const uint64_t orderId) const;
    virtual LimitOrderIndex::const_iterator fetchLimitOrderIterator(const uint64_t orderId) const;
    virtual MarketOrderIndex::const_iterator fetchMarketOrderIterator(const uint64_t orderId) const;

    Utils::Counter::IdHandlerBase myOrderIdHandler = Utils::Counter::IdHandlerBase();
    Utils::Counter::IdHandlerBase myEventIdHandler = Utils::Counter::IdHandlerBase();
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> myWorldClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    std::shared_ptr<Exchange::IMatchingEngine> myMatchingEngine;
    LimitOrderIndex myActiveLimitOrders;
    MarketOrderIndex myQueuedMarketOrders; // empty most of the time
    double myMinimumPriceTick = 0.01;
    long long myMillisecondsToPauseBeforeEventSubmit = 0; // avoids high-frequency log outputs e.g. book snapshot during debug
    bool myTimeEngineOrderEventsProcessing = false; // whether to time order events processing in the matching engine
    bool mySyncClockWithEngine = false;
    bool myDebugMode = false;
    bool myPrintOrderBookPerOrderSubmit = false;
};

std::ostream& operator<<(std::ostream& out, const OrderEventManagerBase& manager);
}

#endif
