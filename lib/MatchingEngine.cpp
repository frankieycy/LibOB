#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine) {
    out << matchingEngine.getAsJason();
    return out;
}

const std::pair<const PriceLevel, uint32_t> IMatchingEngine::getBestBidPriceAndSize() const {
    if (myBidBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myBidBookSize.begin();
}

const std::pair<const PriceLevel, uint32_t> IMatchingEngine::getBestAskPriceAndSize() const {
    if (myAskBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myAskBookSize.begin();
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestBidTopOrder() const {
    if (myBidBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myBidBook.begin();
    return {it->first, it->second.front()};
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> IMatchingEngine::getBestAskTopOrder() const {
    if (myAskBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myAskBook.begin();
    return {it->first, it->second.front()};
}

const double IMatchingEngine::getBestBidPrice() const {
    if (myBidBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myBidBookSize.begin()->first;
}

const double IMatchingEngine::getBestAskPrice() const {
    if (myAskBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myAskBookSize.begin()->first;
}

const double IMatchingEngine::getSpread() const {
    return getBestAskPrice() - getBestBidPrice();
}

const double IMatchingEngine::getHalfSpread() const {
    return getSpread() / 2.0;
}

const double IMatchingEngine::getMidPrice() const {
    return (getBestBidPrice() + getBestAskPrice()) / 2.0;
}

const double IMatchingEngine::getMicroPrice() const {
    return (getBestBidPrice() * getBestAskSize() + getBestAskPrice() * getBestBidSize()) / (getBestBidSize() + getBestAskSize());
}

const double IMatchingEngine::getOrderImbalance() const {
    return (getBestBidSize() - getBestAskSize()) / (getBestBidSize() + getBestAskSize());
}

const double IMatchingEngine::getLastTradePrice() const {
    return getLastTrade()->getPrice();
}

const uint32_t IMatchingEngine::getBestBidSize() const {
    if (myBidBookSize.empty())
        return 0;
    return myBidBookSize.begin()->second;
}

const uint32_t IMatchingEngine::getBestAskSize() const {
    if (myAskBookSize.empty())
        return 0;
    return myAskBookSize.begin()->second;
}

const uint32_t IMatchingEngine::getBidSize(const PriceLevel& priceLevel) const {
    const auto& it = myBidBookSize.find(priceLevel);
    return it != myBidBookSize.end() ? it->second : 0;
}

const uint32_t IMatchingEngine::getAskSize(const PriceLevel& priceLevel) const {
    const auto& it = myAskBookSize.find(priceLevel);
    return it != myAskBookSize.end() ? it->second : 0;
}

const uint32_t IMatchingEngine::getLastTradeSize() const {
    return getLastTrade()->getQuantity();
}

const size_t IMatchingEngine::getNumberOfBidPriceLevels() const {
    return myBidBook.size();
}

const size_t IMatchingEngine::getNumberOfAskPriceLevels() const {
    return myAskBook.size();
}

const size_t IMatchingEngine::getNumberOfTrades() const {
    return myTradeLog.size();
}

const std::shared_ptr<Market::TradeBase> IMatchingEngine::getLastTrade() const {
    if (myTradeLog.empty())
        return nullptr;
    return myTradeLog.back();
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderBase>& order) {
    if (!order)
        Error::LIB_THROW("IMatchingEngine::process: order is null.");
    order->submit(*this);
}

void IMatchingEngine::process(const std::shared_ptr<Market::OrderEventBase>& event) {
    if (!event)
        Error::LIB_THROW("IMatchingEngine::process: order event is null.");
    if (event->isSubmit())
        process(event->getOrder());
    const auto& it = myLimitOrderLookup.find(event->getOrderId());
    if (it != myLimitOrderLookup.end())
        (*it->second)->executeOrderEvent(*event);
}

std::ostream& IMatchingEngine::orderBookSnapshot(std::ostream& out) const {
    // TODO
    return out;
}

void IMatchingEngine::init() {
    // TODO
}

void IMatchingEngine::reset() {
    mySymbol.clear();
    myExchangeId.clear();
    myBidBook.clear();
    myAskBook.clear();
    myBidBookSize.clear();
    myAskBookSize.clear();
    myMarketQueue.clear();
    myTradeLog.clear();
    myLimitOrderLookup.clear();
}

const std::string IMatchingEngine::getAsJason() const {
    // TODO
    return "";
}

void MatchingEngineFIFO::addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) {
    // TODO
    std::cout << "Add to limit order book: " << *order << std::endl;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    const PriceLevel price = order->getPrice();
    const uint32_t originalQuantity = order->getQuantity();
    uint32_t unfilledQuantity = order->getQuantity();
    DescOrderBook& bidBook = getBidBook();
    DescOrderBookSize& bidBookSize = getBidBookSize();
    AscOrderBook& askBook = getAskBook();
    AscOrderBookSize& askBookSize = getAskBookSize();
    TradeLog& tradeLog = getTradeLog();
    RemovedLimitOrderLog& removedLimitOrderLog = getRemovedLimitOrderLog();
    OrderIndex& limitOrderLookup = getLimitOrderLookup();
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty() && price >= askBook.begin()->first) {
            LimitQueue& askQueue = askBook.begin()->second;
            uint32_t& askSizeTotal = askBookSize.begin()->second;
            auto queueIt = askQueue.begin();
            while (unfilledQuantity && queueIt != askQueue.end()) {
                auto& askOrder = *queueIt;
                const uint32_t askQuantity = askOrder->getQuantity();
                bool askFullyFilled = false;
                if (askQuantity <= unfilledQuantity) {
                    unfilledQuantity -= askQuantity;
                    askSizeTotal -= askQuantity;
                    askOrder->setQuantity(0);
                    askOrder->setOrderState(Market::OrderState::FILLED);
                    removedLimitOrderLog.push_back(askOrder);
                    limitOrderLookup.erase(askOrder->getId());
                    queueIt = askQueue.erase(queueIt);
                    askFullyFilled = true;
                } else {
                    askOrder->setQuantity(askQuantity - unfilledQuantity);
                    unfilledQuantity = 0;
                }
                tradeLog.push_back(std::make_shared<Market::TradeBase>(0, order->getTimestamp(), order->getId(), askOrder->getId(), askFullyFilled ? askQuantity : unfilledQuantity, askOrder->getPrice(), true, true, true));
            }
        }
        if (unfilledQuantity) {
            order->setQuantity(unfilledQuantity);
            if (unfilledQuantity < originalQuantity)
                order->setOrderState(Market::OrderState::PARTIAL_FILLED);
            LimitQueue& bidQueue = bidBook[price];
            uint32_t& bidSizeTotal = bidBookSize[price];
            bidQueue.push_back(order);
            bidSizeTotal += order->getQuantity();
            limitOrderLookup[order->getId()] = bidQueue.end();
        } else {
            order->setQuantity(0);
            order->setOrderState(Market::OrderState::FILLED);
        }
    }
    else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty() && price <= bidBook.begin()->first) {
            LimitQueue& bidQueue = bidBook.begin()->second;
            uint32_t& bidSizeTotal = bidBookSize.begin()->second;
            auto queueIt = bidQueue.begin();
            while (unfilledQuantity && queueIt != bidQueue.end()) {
                auto& bidOrder = *queueIt;
                const uint32_t bidQuantity = bidOrder->getQuantity();
                bool bidFullyFilled = false;
                if (bidQuantity <= unfilledQuantity) {
                    unfilledQuantity -= bidQuantity;
                    bidSizeTotal -= bidQuantity;
                    bidOrder->setQuantity(0);
                    bidOrder->setOrderState(Market::OrderState::FILLED);
                    removedLimitOrderLog.push_back(bidOrder);
                    limitOrderLookup.erase(bidOrder->getId());
                    queueIt = bidQueue.erase(queueIt);
                    bidFullyFilled = true;
                } else {
                    bidOrder->setQuantity(bidQuantity - unfilledQuantity);
                    unfilledQuantity = 0;
                }
                tradeLog.push_back(std::make_shared<Market::TradeBase>(0, order->getTimestamp(), bidOrder->getId(), order->getId(), bidFullyFilled ? bidQuantity : unfilledQuantity, bidOrder->getPrice(), true, true, false));
            }
        }
        if (unfilledQuantity) {
            order->setQuantity(unfilledQuantity);
            if (unfilledQuantity < originalQuantity)
                order->setOrderState(Market::OrderState::PARTIAL_FILLED);
            LimitQueue& askQueue = askBook[price];
            uint32_t& askSizeTotal = askBookSize[price];
            askQueue.push_back(order);
            askSizeTotal += order->getQuantity();
            limitOrderLookup[order->getId()] = askQueue.end();
        } else {
            order->setQuantity(0);
            order->setOrderState(Market::OrderState::FILLED);
        }
    }
}

void MatchingEngineFIFO::executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) {
    // TODO
    std::cout << "Execute market order: " << *order << std::endl;
    if (!order->isAlive())
        return;
}

void MatchingEngineFIFO::init() {
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
