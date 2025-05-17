#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils.hpp"
#include "OrderEvent.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"

namespace Exchange {
using PriceLevel = double;
using LimitQueue = std::list<std::shared_ptr<Market::LimitOrder>>;
using MarketQueue = std::list<std::shared_ptr<Market::MarketOrder>>;
using TradeLog = std::vector<std::shared_ptr<Market::TradeBase>>;
using RemovedLimitOrderLog = std::vector<std::shared_ptr<Market::LimitOrder>>;
using DescOrderBook = std::map<PriceLevel, LimitQueue, std::greater<double>>;
using AscOrderBook = std::map<PriceLevel, LimitQueue>;
using DescOrderBookSize = std::map<PriceLevel, uint32_t, std::greater<double>>;
using AscOrderBookSize = std::map<PriceLevel, uint32_t>;
using OrderIndex = std::unordered_map<uint64_t, std::pair<LimitQueue*, LimitQueue::iterator>>;

struct OrderExecutionReport;

class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine) = default;
    IMatchingEngine(const bool debugMode);
    const std::string& getSymbol() const { return mySymbol; }
    const std::string& getExchangeId() const { return myExchangeId; }
    const OrderMatchingStrategy getOrderMatchingStrategy() const { return myOrderMatchingStrategy; }
    const OrderBookDisplayConfig& getOrderBookDisplayConfig() const { return myOrderBookDisplayConfig; }
    const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& getWorldClock() const { return myWorldClock; }
    const std::shared_ptr<Utils::Logger::LoggerBase>& getLogger() const { return myLogger; }
    const bool isDebugMode() const { return myDebugMode; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    void setOrderMatchingStrategy(const OrderMatchingStrategy orderMatchingStrategy) { myOrderMatchingStrategy = orderMatchingStrategy; }
    void setOrderBookDisplayConfig(const OrderBookDisplayConfig& orderBookDisplayConfig) { myOrderBookDisplayConfig = orderBookDisplayConfig; }
    void setWorldClock(const std::shared_ptr<Utils::Counter::TimestampHandlerBase>& worldClock) { myWorldClock = worldClock; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; myOrderBookDisplayConfig.setDebugMode(debugMode); }
    void setLogger(const std::shared_ptr<Utils::Logger::LoggerBase>& logger) { myLogger = logger; }
    const uint64_t generateTradeId() { return myTradeIdHandler.generateId(); }
    const uint64_t clockTick(const uint64_t elapsedTimeUnit = 1) { return myWorldClock->tick(elapsedTimeUnit); }
    virtual const double getBestBidPrice() const = 0;
    virtual const double getBestAskPrice() const = 0;
    virtual const double getSpread() const = 0;
    virtual const double getHalfSpread() const = 0;
    virtual const double getMidPrice() const = 0;
    virtual const double getMicroPrice() const = 0;
    virtual const double getOrderImbalance() const = 0;
    virtual const double getLastTradePrice() const = 0;
    virtual const uint32_t getBestBidSize() const = 0;
    virtual const uint32_t getBestAskSize() const = 0;
    virtual const uint32_t getBidSize(const PriceLevel& priceLevel) const = 0;
    virtual const uint32_t getAskSize(const PriceLevel& priceLevel) const = 0;
    virtual const uint32_t getLastTradeSize() const = 0;
    virtual const size_t getNumberOfBidPriceLevels() const = 0;
    virtual const size_t getNumberOfAskPriceLevels() const = 0;
    virtual const size_t getNumberOfTrades() const = 0;
    virtual const std::shared_ptr<Market::TradeBase> getLastTrade() const = 0;
    virtual std::shared_ptr<IMatchingEngine> clone() const = 0;
    virtual void process(const std::shared_ptr<Market::OrderBase>& order) = 0;
    virtual void process(const std::shared_ptr<Market::OrderEventBase>& event) = 0; // OrderEventManager comminucates with MatchingEngine via this process method
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) = 0;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) = 0;
    virtual void setOrderExecutionCallback(std::function<void(const OrderExecutionReport&)> callback) = 0;
    virtual void init() = 0;
    virtual void reset();
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const = 0;
    virtual const std::string getAsJson() const = 0;
protected:
    Utils::Counter::IdHandlerBase& getTradeIdHandler() { return myTradeIdHandler; }
private:
    std::string mySymbol;
    std::string myExchangeId;
    OrderMatchingStrategy myOrderMatchingStrategy = OrderMatchingStrategy::NULL_ORDER_MATCHING_STRATEGY;
    OrderBookDisplayConfig myOrderBookDisplayConfig = OrderBookDisplayConfig();
    Utils::Counter::IdHandlerBase myTradeIdHandler = Utils::Counter::IdHandlerBase();
    std::shared_ptr<Utils::Counter::TimestampHandlerBase> myWorldClock = std::make_shared<Utils::Counter::TimestampHandlerBase>(); // maintains an internal clock that restamps orders upon order events (submit, cancel, etc.)
    std::shared_ptr<Utils::Logger::LoggerBase> myLogger = std::make_shared<Utils::Logger::LoggerBase>();
    bool myDebugMode = false;
};

class MatchingEngineBase : public IMatchingEngine {
public:
    MatchingEngineBase() = default;
    MatchingEngineBase(const MatchingEngineBase& matchingEngine) = default;
    MatchingEngineBase(const bool debugMode) : IMatchingEngine(debugMode) {}
    const DescOrderBook& getBidBook() const { return myBidBook; }
    const AscOrderBook& getAskBook() const { return myAskBook; }
    const DescOrderBookSize& getBidBookSize() const { return myBidBookSize; }
    const AscOrderBookSize& getAskBookSize() const { return myAskBookSize; }
    const MarketQueue& getMarketQueue() const { return myMarketQueue; }
    const TradeLog& getTradeLog() const { return myTradeLog; }
    const RemovedLimitOrderLog& getRemovedLimitOrderLog() const { return myRemovedLimitOrderLog; }
    const OrderIndex& getLimitOrderLookup() const { return myLimitOrderLookup; }
    void setBidBook(const DescOrderBook& bidBook) { myBidBook = bidBook; }
    void setAskBook(const AscOrderBook& askBook) { myAskBook = askBook; }
    void setMarketQueue(const MarketQueue& marketQueue) { myMarketQueue = marketQueue; }
    void setTradeLog(const TradeLog& tradeLog) { myTradeLog = tradeLog; }
    void setRemovedLimitOrderLog(const RemovedLimitOrderLog& removedLimitOrderLog) { myRemovedLimitOrderLog = removedLimitOrderLog; }
    void setLimitOrderLookup(const OrderIndex& limitOrderLookup) { myLimitOrderLookup = limitOrderLookup; }
    const std::pair<const PriceLevel, uint32_t> getBestBidPriceAndSize() const;
    const std::pair<const PriceLevel, uint32_t> getBestAskPriceAndSize() const;
    const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> getBestBidTopOrder() const;
    const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> getBestAskTopOrder() const;
    const double getBestBidPrice() const;
    const double getBestAskPrice() const;
    const double getSpread() const;
    const double getHalfSpread() const;
    const double getMidPrice() const;
    const double getMicroPrice() const;
    const double getOrderImbalance() const;
    const double getLastTradePrice() const;
    const uint32_t getBestBidSize() const;
    const uint32_t getBestAskSize() const;
    const uint32_t getBidSize(const PriceLevel& priceLevel) const;
    const uint32_t getAskSize(const PriceLevel& priceLevel) const;
    const uint32_t getLastTradeSize() const;
    const size_t getNumberOfBidPriceLevels() const;
    const size_t getNumberOfAskPriceLevels() const;
    const size_t getNumberOfTrades() const;
    const std::shared_ptr<Market::TradeBase> getLastTrade() const;
    virtual void process(const std::shared_ptr<Market::OrderBase>& order);
    virtual void process(const std::shared_ptr<Market::OrderEventBase>& event);
    virtual void fillOrderByMatchingTopLimitQueue(const std::shared_ptr<Market::OrderBase>& order, uint32_t& unfilledQuantity, uint32_t& matchSizeTotal, LimitQueue& matchQueue);
    virtual void placeLimitOrderToLimitOrderBook(std::shared_ptr<Market::LimitOrder>& order, const uint32_t unfilledQuantity, uint32_t& orderSizeTotal, LimitQueue& limitQueue);
    virtual void placeMarketOrderToMarketOrderQueue(std::shared_ptr<Market::MarketOrder>& order, const uint32_t unfilledQuantity, MarketQueue& marketQueue);
    virtual void setOrderExecutionCallback(std::function<void(const OrderExecutionReport&)> callback) { this->myOrderExecutionCallback = callback; }
    virtual void init();
    virtual void reset();
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const;
    virtual const std::string getAsJson() const;
protected:
    DescOrderBook& getBidBook() { return myBidBook; }
    AscOrderBook& getAskBook() { return myAskBook; }
    DescOrderBookSize& getBidBookSize() { return myBidBookSize; }
    AscOrderBookSize& getAskBookSize() { return myAskBookSize; }
    MarketQueue& getMarketQueue() { return myMarketQueue; }
    TradeLog& getTradeLog() { return myTradeLog; }
    RemovedLimitOrderLog& getRemovedLimitOrderLog() { return myRemovedLimitOrderLog; }
    OrderIndex& getLimitOrderLookup() { return myLimitOrderLookup; }
private:
    // each order processing must remember to update ALL the following data structures at once, each operation takes O(1)
    DescOrderBook myBidBook;
    AscOrderBook myAskBook;
    DescOrderBookSize myBidBookSize;
    AscOrderBookSize myAskBookSize;
    MarketQueue myMarketQueue;
    TradeLog myTradeLog;
    RemovedLimitOrderLog myRemovedLimitOrderLog;
    OrderIndex myLimitOrderLookup;
    std::function<void(const OrderExecutionReport&)> myOrderExecutionCallback;
};

class MatchingEngineFIFO : public MatchingEngineBase {
public:
    MatchingEngineFIFO() = default;
    MatchingEngineFIFO(const MatchingEngineFIFO& matchingEngine) = default;
    MatchingEngineFIFO(const bool debugMode) : MatchingEngineBase(debugMode) {}
    virtual std::shared_ptr<IMatchingEngine> clone() const override { return std::make_shared<MatchingEngineFIFO>(*this); }
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) override;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) override;
    virtual void init() override;
};

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine);
}

#endif
