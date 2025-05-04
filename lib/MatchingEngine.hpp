#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils.hpp"
#include "OrderEvent.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"

namespace Exchange {
using PriceLevel = double;
using LimitQueue = std::deque<std::shared_ptr<Market::LimitOrder>>;
using MarketQueue = std::deque<std::shared_ptr<Market::MarketOrder>>;
using TradeLog = std::vector<std::shared_ptr<Market::TradeBase>>;
using DescOrderBook = std::map<PriceLevel, LimitQueue, std::greater<double>>;
using AscOrderBook = std::map<PriceLevel, LimitQueue>;
using DescOrderBookSize = std::map<PriceLevel, int, std::greater<double>>;
using AscOrderBookSize = std::map<PriceLevel, int>;
using OrderIndex = std::unordered_map<uint64_t, std::shared_ptr<Market::LimitOrder>>;

class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine) = default;
    const std::string& getSymbol() const { return mySymbol; }
    const std::string& getExchangeId() const { return myExchangeId; }
    const DescOrderBook& getBidBook() const { return myBidBook; }
    const AscOrderBook& getAskBook() const { return myAskBook; }
    const DescOrderBookSize& getBidBookSize() const { return myBidBookSize; }
    const AscOrderBookSize& getAskBookSize() const { return myAskBookSize; }
    const MarketQueue& getMarketQueue() const { return myMarketQueue; }
    const TradeLog& getTradeLog() const { return myTradeLog; }
    const OrderIndex& getLimitOrderLookup() const { return myLimitOrderLookup; }
    const OrderMatchingStrategy getOrderMatchingStrategy() const { return myOrderMatchingStrategy; }
    void setSymbol(const std::string& symbol) { mySymbol = symbol; }
    void setExchangeId(const std::string& exchangeId) { myExchangeId = exchangeId; }
    void setBidBook(const DescOrderBook& bidBook) { myBidBook = bidBook; }
    void setAskBook(const AscOrderBook& askBook) { myAskBook = askBook; }
    void setMarketQueue(const MarketQueue& marketQueue) { myMarketQueue = marketQueue; }
    void setTradeLog(const TradeLog& tradeLog) { myTradeLog = tradeLog; }
    void setLimitOrderLookup(const OrderIndex& limitOrderLookup) { myLimitOrderLookup = limitOrderLookup; }
    void setOrderMatchingStrategy(const OrderMatchingStrategy orderMatchingStrategy) { myOrderMatchingStrategy = orderMatchingStrategy; }
    const std::pair<const PriceLevel, int> getBestBidPriceAndSize() const;
    const std::pair<const PriceLevel, int> getBestAskPriceAndSize() const;
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
    const int getBestBidSize() const;
    const int getBestAskSize() const;
    const int getBidSize(const PriceLevel& priceLevel) const;
    const int getAskSize(const PriceLevel& priceLevel) const;
    const int getLastTradeSize() const;
    const int getNumberOfBidPriceLevels() const;
    const int getNumberOfAskPriceLevels() const;
    const int getNumberOfTrades() const;
    const std::shared_ptr<Market::TradeBase>& getLastTrade() const;
    virtual std::shared_ptr<IMatchingEngine> clone() const = 0;
    virtual void process(const std::shared_ptr<Market::OrderBase>& order);
    virtual void process(const std::shared_ptr<Market::OrderEventBase>& event);
    virtual void addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) = 0;
    virtual void executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) = 0;
    virtual void init();
    virtual void reset();
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const;
    virtual const std::string getAsJason() const;
    friend std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine);
private:
    std::string mySymbol;
    std::string myExchangeId;
    DescOrderBook myBidBook;
    AscOrderBook myAskBook;
    DescOrderBookSize myBidBookSize;
    AscOrderBookSize myAskBookSize;
    MarketQueue myMarketQueue;
    TradeLog myTradeLog;
    OrderIndex myLimitOrderLookup;
    OrderMatchingStrategy myOrderMatchingStrategy = OrderMatchingStrategy::NULL_ORDER_MATCHING_STRATEGY;
};

class MatchingEngineFIFO : public IMatchingEngine {
public:
    MatchingEngineFIFO() = default;
    MatchingEngineFIFO(const MatchingEngineFIFO& matchingEngine) = default;
    virtual std::shared_ptr<IMatchingEngine> clone() const override { return std::make_shared<MatchingEngineFIFO>(*this); }
    virtual void addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) override;
    virtual void executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) override;
    virtual void init() override;
};
}

#endif
