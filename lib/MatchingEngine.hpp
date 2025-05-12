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

class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine) = default;
    IMatchingEngine(const bool debugMode) : myDebugMode(debugMode), myOrderBookDisplayConfig(OrderBookDisplayConfig(debugMode)) {}
    const std::string& getSymbol() const { return mySymbol; }
    const std::string& getExchangeId() const { return myExchangeId; }
    const OrderMatchingStrategy getOrderMatchingStrategy() const { return myOrderMatchingStrategy; }
    const OrderBookDisplayConfig& getOrderBookDisplayConfig() const { return myOrderBookDisplayConfig; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    void setOrderMatchingStrategy(const OrderMatchingStrategy orderMatchingStrategy) { myOrderMatchingStrategy = orderMatchingStrategy; }
    void setOrderBookDisplayConfig(const OrderBookDisplayConfig& orderBookDisplayConfig) { myOrderBookDisplayConfig = orderBookDisplayConfig; }
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
    virtual void process(const std::shared_ptr<Market::OrderEventBase>& event) = 0;
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) = 0;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) = 0;
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
    DescOrderBook myBidBook;
    AscOrderBook myAskBook;
    DescOrderBookSize myBidBookSize;
    AscOrderBookSize myAskBookSize;
    MarketQueue myMarketQueue;
    TradeLog myTradeLog;
    RemovedLimitOrderLog myRemovedLimitOrderLog;
    OrderIndex myLimitOrderLookup;
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
