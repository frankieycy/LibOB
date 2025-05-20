#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"
#include "Market/Order.hpp"
#include "Market/Trade.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

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
using OrderIndex = std::unordered_map<uint64_t, std::pair<LimitQueue*, LimitQueue::iterator>>; // permits O(1) order access for cancellation and modification

enum class OrderExecutionType { FILLED, PARTIAL_FILLED, CANCELLED, REJECTED, NULL_ORDER_EXECUTION_TYPE };
enum class OrderOperationType { CANCEL, MODIFY_PRICE, MODIFY_QUANTITY, NULL_ORDER_OPERATION_TYPE };
enum class OrderOperationStatus { SUCCESS, FAILURE, NULL_ORDER_OPERATION_STATUS };

// NASDAQ ITCH-like compact order reports disseminated to the OrderEventManager
struct OrderExecutionReport {
    uint64_t timestamp;
    uint64_t orderId;
    uint64_t tradeId;
    uint32_t filledQuantity;
    double filledPrice;
    bool isMakerOrder; // if the order is a resting maker order
    Market::Side orderSide;
    OrderExecutionType orderExecutionType;
    std::optional<uint64_t> latency = std::nullopt;
    std::shared_ptr<Market::TradeBase> trade = nullptr;
};

struct OrderSubmissionReport;

struct OrderOperationReport {
    uint64_t timestamp;
    uint64_t orderId;
    Market::Side orderSide;
    OrderOperationType orderOperationType;
    OrderOperationStatus status = OrderOperationStatus::SUCCESS;
    std::optional<uint32_t> modifiedQuantity = std::nullopt;
    std::optional<double> modifiedPrice = std::nullopt;
    std::optional<std::string> message = std::nullopt;
};

class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine) = default;
    IMatchingEngine(const bool debugMode);
    virtual ~IMatchingEngine() = default;
    std::string getSymbol() const { return mySymbol; }
    std::string getExchangeId() const { return myExchangeId; }
    OrderMatchingStrategy getOrderMatchingStrategy() const { return myOrderMatchingStrategy; }
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
    virtual std::shared_ptr<Market::TradeBase> getLastTrade() const = 0;
    virtual std::shared_ptr<IMatchingEngine> clone() const = 0;
    virtual void process(const std::shared_ptr<const Market::OrderBase>& order) = 0;
    virtual void process(const std::shared_ptr<const Market::OrderEventBase>& event) = 0; // OrderEventManager comminucates with MatchingEngine via this process method
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) = 0;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) = 0;
    virtual void setOrderExecutionCallback(std::function<void(const OrderExecutionReport&)> callback) = 0;
    virtual void init() = 0;
    virtual void reset();
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const = 0;
    virtual std::string getAsJson() const = 0;
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

/* A simple MatchingEngine implmentation with O(1) order operations, including
   submission, cancellation, modification, BBO fetch etc. */
class MatchingEngineBase : public IMatchingEngine {
public:
    MatchingEngineBase() = default;
    MatchingEngineBase(const MatchingEngineBase& matchingEngine) = default;
    MatchingEngineBase(const bool debugMode) : IMatchingEngine(debugMode) {}
    virtual ~MatchingEngineBase() = default;
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
    std::shared_ptr<Market::TradeBase> getLastTrade() const override;
    virtual void process(const std::shared_ptr<const Market::OrderBase>& order) override;
    virtual void process(const std::shared_ptr<const Market::OrderEventBase>& event) override;
    virtual void fillOrderByMatchingTopLimitQueue(const std::shared_ptr<Market::OrderBase>& order, uint32_t& unfilledQuantity, uint32_t& matchSizeTotal, LimitQueue& matchQueue);
    virtual void placeLimitOrderToLimitOrderBook(std::shared_ptr<Market::LimitOrder>& order, const uint32_t unfilledQuantity, uint32_t& orderSizeTotal, LimitQueue& limitQueue);
    virtual void placeMarketOrderToMarketOrderQueue(std::shared_ptr<Market::MarketOrder>& order, const uint32_t unfilledQuantity, MarketQueue& marketQueue);
    virtual void setOrderExecutionCallback(std::function<void(const OrderExecutionReport&)> callback) override { this->myOrderExecutionCallback = callback; }
    virtual void init() override;
    virtual void reset() override;
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const override;
    virtual std::string getAsJson() const override;
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
    virtual ~MatchingEngineFIFO() = default;
    virtual std::shared_ptr<IMatchingEngine> clone() const override { return std::make_shared<MatchingEngineFIFO>(*this); }
    virtual void addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) override;
    virtual void executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) override;
    virtual void init() override;
};

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine);
}

#endif
