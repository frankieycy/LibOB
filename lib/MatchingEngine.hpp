#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"

namespace Exchange {
class MatchingEngineBase {
public:
    MatchingEngineBase() = default;
    MatchingEngineBase(const MatchingEngineBase& matchingEngine);
    virtual std::shared_ptr<MatchingEngineBase> clone() const { return std::make_shared<MatchingEngineBase>(*this); }
    virtual void process(const std::shared_ptr<Market::OrderBase>& order);
    virtual void addToLimitOrderBook(const std::shared_ptr<Market::LimitOrder>& order);
    virtual void executeMarketOrder(const std::shared_ptr<Market::MarketOrder>& order);
    virtual void init();
    virtual const std::string getAsJason() const;
private:
    std::string mySymbol;
    std::string myExchangeId;
    std::map<double, std::deque<const std::shared_ptr<Market::LimitOrder>>, std::greater<double>> myBidLimitOrderBook;
    std::map<double, std::deque<const std::shared_ptr<Market::LimitOrder>>> myAskLimitOrderBook;
    std::deque<Market::MarketOrder> myMarketOrderQueue;
    std::vector<Market::TradeBase> myTradesLog;
};
}

#endif
