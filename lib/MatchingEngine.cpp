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
    matchingEngine.orderBookSnapshot(out);
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
    if (it != myLimitOrderLookup.end()) {
        auto& queueOrderPair = it->second;
        auto& queue = queueOrderPair.first;
        auto& orderIt = queueOrderPair.second;
        auto& order = *orderIt;
        order->executeOrderEvent(*event);
        if (!order->isAlive()) {
            myRemovedLimitOrderLog.push_back(order);
            myLimitOrderLookup.erase(it);
            queue->erase(orderIt);
        }
    }
}

std::ostream& IMatchingEngine::orderBookSnapshot(std::ostream& out) const {
    out << "================= Order Book Snapshot ===================\n";
    out << "  BID Size | BID Price || Level || ASK Price | ASK Size  \n";
    out << "---------------------------------------------------------\n";

    auto bidIt = myBidBook.begin();
    auto askIt = myAskBook.begin();
    uint level = 1;

    if (myOrderBookDisplayConfig.isShowOrderBook()) {
        if (myOrderBookDisplayConfig.isAggregateOrderBook()) {
            while (bidIt != myBidBook.end() || askIt != myAskBook.end()) {
                if (bidIt != myBidBook.end()) {
                    uint32_t bidSize = 0;
                    for (const auto& order : bidIt->second)
                        bidSize += order->getQuantity();
                    out << std::setw(9) << bidSize << "  | "
                        << std::fixed << std::setprecision(2)
                        << std::setw(8) << bidIt->first << "  || ";
                    ++bidIt;
                } else {
                    out << "           |           || ";
                }
                out << std::setw(5) << level << " || ";
                if (askIt != myAskBook.end()) {
                    uint32_t askSize = 0;
                    for (const auto& order : askIt->second)
                        askSize += order->getQuantity();
                    out << std::setw(8) << askIt->first << "  | "
                        << std::setw(8) << askSize << "  \n";
                    ++askIt;
                } else {
                    out << "          |           \n";
                }
                if (++level > myOrderBookDisplayConfig.getOrderBookLevels())
                    break;
            }
        } else {
            // TODO
        }
    }

    out << "---------------------------------------------------------\n";
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
    myRemovedLimitOrderLog.clear();
    myLimitOrderLookup.clear();
}

const std::string IMatchingEngine::getAsJason() const {
    // TODO
    return "";
}

void fillOrderByMatchingLimitQueue(
    const std::shared_ptr<Market::OrderBase>& order,
    uint32_t& unfilledQuantity,
    uint32_t& matchSizeTotal,
    LimitQueue& matchQueue,
    TradeLog& tradeLog,
    RemovedLimitOrderLog& removedLimitOrderLog,
    OrderIndex& limitOrderLookup) {
    const uint64_t orderId = order->getId();
    const bool isIncomingOrderBuy = order->isBuy();
    auto queueIt = matchQueue.begin();
    while (unfilledQuantity && queueIt != matchQueue.end()) {
        auto& matchOrder = *queueIt;
        std::cout << "DEBUG: Matching order: " << *matchOrder << std::endl;
        const uint32_t matchQuantity = matchOrder->getQuantity();
        bool matchFullyFilled = false;
        if (matchQuantity <= unfilledQuantity) {
            unfilledQuantity -= matchQuantity;
            matchSizeTotal -= matchQuantity;
            matchOrder->setQuantity(0);
            matchOrder->setOrderState(Market::OrderState::FILLED);
            removedLimitOrderLog.push_back(matchOrder);
            limitOrderLookup.erase(matchOrder->getId());
            queueIt = matchQueue.erase(queueIt);
            matchFullyFilled = true;
        } else {
            matchOrder->setQuantity(matchQuantity - unfilledQuantity);
            unfilledQuantity = 0;
        }
        if (isIncomingOrderBuy)
            tradeLog.push_back(std::make_shared<Market::TradeBase>(0, order->getTimestamp(), order->getId(), matchOrder->getId(), matchFullyFilled ? matchQuantity : unfilledQuantity, matchOrder->getPrice(), true, true, true));
        else
            tradeLog.push_back(std::make_shared<Market::TradeBase>(0, order->getTimestamp(), matchOrder->getId(), order->getId(), matchFullyFilled ? matchQuantity : unfilledQuantity, matchOrder->getPrice(), true, true, false));
    }
}

void placeLimitOrderToLimitOrderBook(
    std::shared_ptr<Market::LimitOrder>& order,
    const uint32_t unfilledQuantity,
    uint32_t& orderSizeTotal,
    LimitQueue& limitQueue,
    OrderIndex& orderLookup) {
    const double price = order->getPrice();
    const uint32_t originalQuantity = order->getQuantity();
    if (unfilledQuantity) {
        order->setQuantity(unfilledQuantity);
        if (unfilledQuantity < originalQuantity)
            order->setOrderState(Market::OrderState::PARTIAL_FILLED);
        limitQueue.push_back(order);
        orderSizeTotal += order->getQuantity();
        orderLookup[order->getId()] = {&limitQueue, std::prev(limitQueue.end())};
        std::cout << "DEBUG: Placed order in limit order book: " << *order << std::endl;
    } else {
        order->setQuantity(0);
        order->setOrderState(Market::OrderState::FILLED);
    }
}

void placeMarketOrderToMarketOrderQueue(
    std::shared_ptr<Market::MarketOrder>& order,
    const uint32_t unfilledQuantity,
    MarketQueue& marketQueue) {
    const uint32_t originalQuantity = order->getQuantity();
    if (unfilledQuantity) {
        order->setQuantity(unfilledQuantity);
        if (unfilledQuantity < originalQuantity)
            order->setOrderState(Market::OrderState::PARTIAL_FILLED);
        marketQueue.push_back(order);
        std::cout << "DEBUG: Placed order in market order queue: " << *order << std::endl;
    } else {
        order->setQuantity(0);
        order->setOrderState(Market::OrderState::FILLED);
    }
}

void MatchingEngineFIFO::addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) {
    std::cout << "DEBUG: Add to limit order book: " << *order << std::endl;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    const PriceLevel price = order->getPrice();
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
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        placeLimitOrderToLimitOrderBook(order, unfilledQuantity, bidBookSize[price], bidBook[price], limitOrderLookup);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty() && price <= bidBook.begin()->first) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup);
            if (bidBook.begin()->second.empty()) {
                bidBook.erase(bidBook.begin());
                bidBookSize.erase(bidBookSize.begin());
            }
        }
        placeLimitOrderToLimitOrderBook(order, unfilledQuantity, askBookSize[price], askBook[price], limitOrderLookup);
    }
}

void MatchingEngineFIFO::executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) {
    std::cout << "DEBUG: Execute market order: " << *order << std::endl;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    uint32_t unfilledQuantity = order->getQuantity();
    DescOrderBook& bidBook = getBidBook();
    DescOrderBookSize& bidBookSize = getBidBookSize();
    AscOrderBook& askBook = getAskBook();
    AscOrderBookSize& askBookSize = getAskBookSize();
    MarketQueue& marketQueue = getMarketQueue();
    TradeLog& tradeLog = getTradeLog();
    RemovedLimitOrderLog& removedLimitOrderLog = getRemovedLimitOrderLog();
    OrderIndex& limitOrderLookup = getLimitOrderLookup();
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty()) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        placeMarketOrderToMarketOrderQueue(order, unfilledQuantity, marketQueue);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty()) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup);
            if (bidBook.begin()->second.empty()) {
                bidBook.erase(bidBook.begin());
                bidBookSize.erase(bidBookSize.begin());
            }
        }
        placeMarketOrderToMarketOrderQueue(order, unfilledQuantity, marketQueue);
    }
}

void MatchingEngineFIFO::init() {
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
