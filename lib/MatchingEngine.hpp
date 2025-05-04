#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"

namespace Exchange {
class IMatchingEngine {
public:
    IMatchingEngine() = default;
    IMatchingEngine(const IMatchingEngine& matchingEngine) = default;
    virtual std::shared_ptr<IMatchingEngine> clone() const = 0;
    virtual void process(const std::shared_ptr<Market::OrderBase>& order);
    virtual void addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) = 0;
    virtual void executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) = 0;
    virtual void init() {};
    virtual std::ostream& orderBookSnapshot(std::ostream& out) const;
    virtual const std::string getAsJason() const;
    friend std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine);
private:
    using PriceLevel = double;
    using LimitQueue = std::deque<std::shared_ptr<Market::LimitOrder>>;
    using DescOrderBook = std::map<PriceLevel, LimitQueue, std::greater<double>>;
    using AscOrderBook = std::map<PriceLevel, LimitQueue>;
    std::string mySymbol;
    std::string myExchangeId;
    DescOrderBook myBidBook;
    AscOrderBook myAskBook;
    std::deque<std::shared_ptr<Market::MarketOrder>> myMarketOrders;
    std::vector<std::shared_ptr<Market::TradeBase>> myTrades;
};

class MatchingEngineFIFO : public IMatchingEngine {
public:
    MatchingEngineFIFO() = default;
    MatchingEngineFIFO(const MatchingEngineFIFO& matchingEngine) = default;
    virtual std::shared_ptr<IMatchingEngine> clone() const override { return std::make_shared<MatchingEngineFIFO>(*this); }
    virtual void addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order) override {}
    virtual void executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order) override {}
};
}

#endif
