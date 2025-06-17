#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"
#include "Market/Order.hpp"
#include "Market/Trade.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Exchange {
using PriceLevel = double;
using LimitQueue = std::list<std::shared_ptr<Market::LimitOrder>>;
using MarketQueue = std::list<std::shared_ptr<Market::MarketOrder>>;
using TradeLog = std::vector<std::shared_ptr<const Market::TradeBase>>;
using OrderEventLog = std::vector<std::shared_ptr<const Market::OrderEventBase>>;
using OrderProcessingReportLog = std::vector<std::shared_ptr<const OrderProcessingReport>>;
using ITCHMessageLog = std::vector<std::shared_ptr<const ITCHEncoder::ITCHMessage>>;
using RemovedLimitOrderLog = std::vector<std::shared_ptr<const Market::LimitOrder>>;
using DescOrderBook = std::map<PriceLevel, LimitQueue, std::greater<double>>;
using AscOrderBook = std::map<PriceLevel, LimitQueue>;
using DescOrderBookSize = std::map<PriceLevel, uint32_t, std::greater<double>>;
using AscOrderBookSize = std::map<PriceLevel, uint32_t>;
using OrderIndex = std::unordered_map<uint64_t, std::pair<LimitQueue*, LimitQueue::iterator>>; // permits O(1) order access for cancellation and modification
template<typename T>
using CallbackFunction = std::function<void(const std::shared_ptr<const T>&)>;
template<typename T> // use shared_ptr for callback function so that clients can manage its lifetime
using CallbackSharedPtr = std::shared_ptr<CallbackFunction<T>>;
using OrderProcessingCallback = CallbackFunction<OrderProcessingReport>; // communicates with OrderEventManager
using ITCHMessageCallback = CallbackFunction<ITCHEncoder::ITCHMessage>;

class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine);
    IMatchingEngine(const bool debugMode);
    virtual ~IMatchingEngine() = default;
    std::string getSymbol() const { return mySymbol; }
    std::string getExchangeId() const { return myExchangeId; }
    OrderMatchingStrategy getOrderMatchingStrategy() const { return myOrderMatchingStrategy; }
    OrderBookDisplayConfig& getOrderBookDisplayConfig() { return myOrderBookDisplayConfig; }
    const OrderBookDisplayConfig& getOrderBookDisplayConfig() const { return myOrderBookDisplayConfig; }
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> getWorldClock() const { return myWorldClock; }
    std::shared_ptr<Utils::Logger::LoggerBase> getLogger() const { return myLogger; }
    bool isDebugMode() const { return myDebugMode; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    void setOrderMatchingStrategy(const OrderMatchingStrategy orderMatchingStrategy) { myOrderMatchingStrategy = orderMatchingStrategy; }
    void setOrderBookDisplayConfig(const OrderBookDisplayConfig& orderBookDisplayConfig) { myOrderBookDisplayConfig = orderBookDisplayConfig; }
    void setWorldClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& worldClock) { myWorldClock = worldClock; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; myOrderBookDisplayConfig.setDebugMode(debugMode); }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    uint64_t generateTradeId() { return myTradeIdHandler.generateId(); }
    uint64_t generateReportId() { return myReportIdHandler.generateId(); }
    uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return myWorldClock->tick(elapsedTimeUnit); }
    virtual std::pair<const PriceLevel, uint32_t> getBestBidPriceAndSize() const = 0;
    virtual std::pair<const PriceLevel, uint32_t> getBestAskPriceAndSize() const = 0;
    virtual std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> getBestBidTopOrder() const = 0;
    virtual std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> getBestAskTopOrder() const = 0;
    virtual double getBestBidPrice() const = 0;
    virtual double getBestAskPrice() const = 0;
    virtual double getSpread() const = 0;
    virtual double getHalfSpread() const = 0;
    virtual double getMidPrice() const = 0;
    virtual double getMicroPrice() const = 0;
    virtual double getOrderImbalance() const = 0;
    virtual double getLastTradePrice() const = 0;
    virtual uint32_t getBestBidSize() const = 0;
    virtual uint32_t getBestAskSize() const = 0;
    virtual uint32_t getBidSize(const PriceLevel& priceLevel) const = 0;
    virtual uint32_t getAskSize(const PriceLevel& priceLevel) const = 0;
    virtual uint32_t getLastTradeSize() const = 0;
    virtual size_t getNumberOfBidPriceLevels() const = 0;
    virtual size_t getNumberOfAskPriceLevels() const = 0;
    virtual size_t getNumberOfTrades() const = 0;
    virtual std::shared_ptr<const Market::TradeBase> getLastTrade() const = 0;
    virtual std::shared_ptr<IMatchingEngine> clone() const = 0;
    virtual void process(const std::shared_ptr<const Market::OrderBase>& order) = 0;
    virtual void process(const std::shared_ptr<const Market::OrderEventBase>& event) = 0; // OrderEventManager comminucates with MatchingEngine via this process method
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) = 0; // left pure virtual unless the order matching strategy is defined
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) = 0;
    virtual void setOrderProcessingCallback(std::function<void(const std::shared_ptr<const OrderProcessingReport>&)> callback) = 0;
    virtual void setITCHMessageCallback(std::function<void(const std::shared_ptr<const ITCHEncoder::ITCHMessage>&)> callback) = 0;
    virtual void addOrderProcessingCallback(const CallbackSharedPtr<OrderProcessingReport>& callback) = 0;
    virtual void addITCHMessageCallback(const CallbackSharedPtr<ITCHEncoder::ITCHMessage>& callback) = 0;
    virtual void reserve(const size_t numOrdersEstimate) = 0; // reserves memory for various data structures (e.g. vector, unordered_map)
    virtual void stateConsistencyCheck() const = 0; // checks the internal state of the matching engine for consistency
    virtual void init() = 0; // state consistency checks and class flags initialization called in every derived constructor
    virtual void reset(); // empties out the order book, trade log, market queue, world clock, logger, etc.
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const = 0;
    virtual std::string getAsJson() const = 0;
protected:
    Utils::Counter::IdHandlerBase& getTradeIdHandler() { return myTradeIdHandler; }
    Utils::Counter::IdHandlerBase& getReportIdHandler() { return myReportIdHandler; }
private:
    std::string mySymbol;
    std::string myExchangeId;
    OrderMatchingStrategy myOrderMatchingStrategy = OrderMatchingStrategy::NULL_ORDER_MATCHING_STRATEGY;
    OrderBookDisplayConfig myOrderBookDisplayConfig = OrderBookDisplayConfig();
    Utils::Counter::IdHandlerBase myTradeIdHandler = Utils::Counter::IdHandlerBase();
    Utils::Counter::IdHandlerBase myReportIdHandler = Utils::Counter::IdHandlerBase();
    // maintains an internal clock that restamps orders upon order events (submit, cancel, etc.)
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> myWorldClock = std::make_shared<Utils::Counter::TimestampHandlerBase>();
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    bool myDebugMode = false;
};

/* A simple MatchingEngine implmentation with O(1) order operations, including
   submission, cancellation, modification, BBO fetch etc. The order book state
   may be completely re-constructed if one gets the order events log, order reports
   log, or the ITCH messages log. */
class MatchingEngineBase : public IMatchingEngine {
public:
    MatchingEngineBase();
    MatchingEngineBase(const MatchingEngineBase& matchingEngine);
    MatchingEngineBase(const bool debugMode) : IMatchingEngine(debugMode) {}
    MatchingEngineBase(const OrderEventLog& orderEventLog);
    MatchingEngineBase(const OrderProcessingReportLog& orderProcessingReportLog);
    virtual ~MatchingEngineBase() = default;
    const DescOrderBook& getBidBook() const { return myBidBook; }
    const AscOrderBook& getAskBook() const { return myAskBook; }
    const DescOrderBookSize& getBidBookSize() const { return myBidBookSize; }
    const AscOrderBookSize& getAskBookSize() const { return myAskBookSize; }
    const MarketQueue& getMarketQueue() const { return myMarketQueue; }
    const TradeLog& getTradeLog() const { return myTradeLog; }
    const OrderEventLog& getOrderEventLog() const { return myOrderEventLog; }
    const OrderProcessingReportLog& getOrderProcessingReportLog() const { return myOrderProcessingReportLog; }
    const ITCHMessageLog& getITCHMessageLog() const { return myITCHMessageLog; }
    const RemovedLimitOrderLog& getRemovedLimitOrderLog() const { return myRemovedLimitOrderLog; }
    const OrderIndex& getLimitOrderLookup() const { return myLimitOrderLookup; }
    OrderProcessingCallback getOrderProcessingCallback() const { return myOrderProcessingCallback; }
    ITCHMessageCallback getITCHMessageCallback() const { return myITCHMessageCallback; }
    void setBidBook(const DescOrderBook& bidBook) { myBidBook = bidBook; }
    void setAskBook(const AscOrderBook& askBook) { myAskBook = askBook; }
    void setMarketQueue(const MarketQueue& marketQueue) { myMarketQueue = marketQueue; }
    void setTradeLog(const TradeLog& tradeLog) { myTradeLog = tradeLog; }
    void setOrderEventLog(const OrderEventLog& orderEventLog) { myOrderEventLog = orderEventLog; }
    void setOrderProcessingReportLog(const OrderProcessingReportLog& orderProcessingReportLog) { myOrderProcessingReportLog = orderProcessingReportLog; }
    void setITCHMessageLog(const ITCHMessageLog& itchMessageLog) { myITCHMessageLog = itchMessageLog; }
    void setRemovedLimitOrderLog(const RemovedLimitOrderLog& removedLimitOrderLog) { myRemovedLimitOrderLog = removedLimitOrderLog; }
    void setLimitOrderLookup(const OrderIndex& limitOrderLookup) { myLimitOrderLookup = limitOrderLookup; }
    std::pair<const PriceLevel, uint32_t> getBestBidPriceAndSize() const override;
    std::pair<const PriceLevel, uint32_t> getBestAskPriceAndSize() const override;
    std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> getBestBidTopOrder() const override;
    std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> getBestAskTopOrder() const override;
    double getBestBidPrice() const override;
    double getBestAskPrice() const override;
    double getSpread() const override;
    double getHalfSpread() const override;
    double getMidPrice() const override;
    double getMicroPrice() const override;
    double getOrderImbalance() const override;
    double getLastTradePrice() const override;
    uint32_t getBestBidSize() const override;
    uint32_t getBestAskSize() const override;
    uint32_t getBidSize(const PriceLevel& priceLevel) const override;
    uint32_t getAskSize(const PriceLevel& priceLevel) const override;
    uint32_t getLastTradeSize() const override;
    size_t getNumberOfBidPriceLevels() const override;
    size_t getNumberOfAskPriceLevels() const override;
    size_t getNumberOfTrades() const override;
    std::shared_ptr<const Market::TradeBase> getLastTrade() const override;
    virtual void process(const std::shared_ptr<const Market::OrderBase>& order) override;
    virtual void process(const std::shared_ptr<const Market::OrderEventBase>& event) override;
    virtual void build(const OrderEventLog& orderEventLog); // builds the book given some user-input order events stream
    virtual void build(const OrderProcessingReportLog& orderProcessingReportLog);
    virtual void build(const ITCHMessageLog& itchMessageLog);
    virtual void executeAgainstQueuedMarketOrders(const std::shared_ptr<Market::LimitOrder>& order, uint32_t& unfilledQuantity, MarketQueue& marketQueue);
    virtual void fillOrderByMatchingTopLimitQueue(const std::shared_ptr<Market::OrderBase>& order, uint32_t& unfilledQuantity, uint32_t& matchSizeTotal, LimitQueue& matchQueue);
    virtual void placeLimitOrderToLimitOrderBook(std::shared_ptr<Market::LimitOrder>& order, const uint32_t unfilledQuantity, uint32_t& orderSizeTotal, LimitQueue& limitQueue);
    virtual void placeMarketOrderToMarketOrderQueue(std::shared_ptr<Market::MarketOrder>& order, const uint32_t unfilledQuantity, MarketQueue& marketQueue);
    virtual void setOrderProcessingCallback(std::function<void(const std::shared_ptr<const OrderProcessingReport>&)> callback) override { myOrderProcessingCallback = callback; }
    virtual void setITCHMessageCallback(std::function<void(const std::shared_ptr<const ITCHEncoder::ITCHMessage>&)> callback) override { myITCHMessageCallback = callback; }
    virtual void addOrderProcessingCallback(const CallbackSharedPtr<OrderProcessingReport>& callback) override { myOrderProcessingCallbacks.push_back(callback); }
    virtual void addITCHMessageCallback(const CallbackSharedPtr<ITCHEncoder::ITCHMessage>& callback) override { myITCHMessageCallbacks.push_back(callback); }
    virtual void logOrderProcessingReport(const std::shared_ptr<const OrderProcessingReport>& report);
    virtual void reserve(const size_t numOrdersEstimate) override;
    virtual void stateConsistencyCheck() const override;
    virtual void init() override;
    virtual void reset() override;
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const override;
    virtual std::string getAsJson() const override;
protected:
    DescOrderBook& accessBidBook() { return myBidBook; }
    AscOrderBook& accessAskBook() { return myAskBook; }
    DescOrderBookSize& accessBidBookSize() { return myBidBookSize; }
    AscOrderBookSize& accessAskBookSize() { return myAskBookSize; }
    MarketQueue& accessMarketQueue() { return myMarketQueue; }
    TradeLog& accessTradeLog() { return myTradeLog; }
    OrderEventLog& accessOrderEventLog() { return myOrderEventLog; }
    OrderProcessingReportLog& accessOrderProcessingReportLog() { return myOrderProcessingReportLog; }
    ITCHMessageLog& accessITCHMessageLog() { return myITCHMessageLog; }
    RemovedLimitOrderLog& accessRemovedLimitOrderLog() { return myRemovedLimitOrderLog; }
    OrderIndex& accessLimitOrderLookup() { return myLimitOrderLookup; }
private:
    // each order processing must remember to update ALL the following data structures at once, each operation takes O(1)
    DescOrderBook myBidBook;
    AscOrderBook myAskBook;
    DescOrderBookSize myBidBookSize;
    AscOrderBookSize myAskBookSize;
    MarketQueue myMarketQueue; // empty most of the time
    TradeLog myTradeLog;
    OrderEventLog myOrderEventLog;
    OrderProcessingReportLog myOrderProcessingReportLog;
    ITCHMessageLog myITCHMessageLog; // another equivalent representation of the order processing report, trimmed and standardized
    RemovedLimitOrderLog myRemovedLimitOrderLog;
    OrderIndex myLimitOrderLookup;
    // the order processing callback can be as complicated as it gets (e.g. the report routed to various handlers)
    // but the exposed interface must be simple
    OrderProcessingCallback myOrderProcessingCallback;
    ITCHMessageCallback myITCHMessageCallback;
    std::vector<CallbackSharedPtr<OrderProcessingReport>> myOrderProcessingCallbacks;
    std::vector<CallbackSharedPtr<ITCHEncoder::ITCHMessage>> myITCHMessageCallbacks;
};

class MatchingEngineFIFO : public MatchingEngineBase {
public:
    MatchingEngineFIFO();
    MatchingEngineFIFO(const MatchingEngineFIFO& matchingEngine);
    MatchingEngineFIFO(const bool debugMode) : MatchingEngineBase(debugMode) {}
    MatchingEngineFIFO(const OrderProcessingReportLog& orderProcessingReportLog) : MatchingEngineBase(orderProcessingReportLog) {}
    virtual ~MatchingEngineFIFO() = default;
    virtual std::shared_ptr<IMatchingEngine> clone() const override { return std::make_shared<MatchingEngineFIFO>(*this); }
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) override;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) override;
    virtual void init() override;
};

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine);
}

#endif
