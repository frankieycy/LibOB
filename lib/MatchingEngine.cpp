#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils.hpp"
#include "Order.hpp"
#include "Trade.hpp"
#include "MatchingEngineUtils.hpp"
#include "MatchingEngine.hpp"

namespace Exchange {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const MatchingEngineBase& matchingEngine) {
    matchingEngine.orderBookSnapshot(out);
    return out;
}

void IMatchingEngine::reset() {
    mySymbol.clear();
    myExchangeId.clear();
}

const std::pair<const PriceLevel, uint32_t> MatchingEngineBase::getBestBidPriceAndSize() const {
    if (myBidBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myBidBookSize.begin();
}

const std::pair<const PriceLevel, uint32_t> MatchingEngineBase::getBestAskPriceAndSize() const {
    if (myAskBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myAskBookSize.begin();
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> MatchingEngineBase::getBestBidTopOrder() const {
    if (myBidBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myBidBook.begin();
    return {it->first, it->second.front()};
}

const std::pair<const PriceLevel, const std::shared_ptr<Market::LimitOrder>> MatchingEngineBase::getBestAskTopOrder() const {
    if (myAskBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myAskBook.begin();
    return {it->first, it->second.front()};
}

const double MatchingEngineBase::getBestBidPrice() const {
    if (myBidBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myBidBookSize.begin()->first;
}

const double MatchingEngineBase::getBestAskPrice() const {
    if (myAskBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myAskBookSize.begin()->first;
}

const double MatchingEngineBase::getSpread() const {
    return getBestAskPrice() - getBestBidPrice();
}

const double MatchingEngineBase::getHalfSpread() const {
    return getSpread() / 2.0;
}

const double MatchingEngineBase::getMidPrice() const {
    return (getBestBidPrice() + getBestAskPrice()) / 2.0;
}

const double MatchingEngineBase::getMicroPrice() const {
    return (getBestBidPrice() * getBestAskSize() + getBestAskPrice() * getBestBidSize()) / (getBestBidSize() + getBestAskSize());
}

const double MatchingEngineBase::getOrderImbalance() const {
    return (getBestBidSize() - getBestAskSize()) / (getBestBidSize() + getBestAskSize());
}

const double MatchingEngineBase::getLastTradePrice() const {
    return getLastTrade()->getPrice();
}

const uint32_t MatchingEngineBase::getBestBidSize() const {
    if (myBidBookSize.empty())
        return 0;
    return myBidBookSize.begin()->second;
}

const uint32_t MatchingEngineBase::getBestAskSize() const {
    if (myAskBookSize.empty())
        return 0;
    return myAskBookSize.begin()->second;
}

const uint32_t MatchingEngineBase::getBidSize(const PriceLevel& priceLevel) const {
    const auto& it = myBidBookSize.find(priceLevel);
    return it != myBidBookSize.end() ? it->second : 0;
}

const uint32_t MatchingEngineBase::getAskSize(const PriceLevel& priceLevel) const {
    const auto& it = myAskBookSize.find(priceLevel);
    return it != myAskBookSize.end() ? it->second : 0;
}

const uint32_t MatchingEngineBase::getLastTradeSize() const {
    return getLastTrade()->getQuantity();
}

const size_t MatchingEngineBase::getNumberOfBidPriceLevels() const {
    return myBidBook.size();
}

const size_t MatchingEngineBase::getNumberOfAskPriceLevels() const {
    return myAskBook.size();
}

const size_t MatchingEngineBase::getNumberOfTrades() const {
    return myTradeLog.size();
}

const std::shared_ptr<Market::TradeBase> MatchingEngineBase::getLastTrade() const {
    if (myTradeLog.empty())
        return nullptr;
    return myTradeLog.back();
}

void MatchingEngineBase::process(const std::shared_ptr<Market::OrderBase>& order) {
    if (!order)
        Error::LIB_THROW("MatchingEngineBase::process: order is null.");
    order->submit(*this);
}

void MatchingEngineBase::process(const std::shared_ptr<Market::OrderEventBase>& event) {
    if (!event)
        Error::LIB_THROW("MatchingEngineBase::process: order event is null.");
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

std::ostream& MatchingEngineBase::orderBookSnapshot(std::ostream& out) const {
    const OrderBookDisplayConfig& config = getOrderBookDisplayConfig();
    if (config.isShowOrderBook()) {
        auto bidIt = myBidBook.begin();
        auto askIt = myAskBook.begin();
        if (config.isAggregateOrderBook()) {
            uint level = 1;
            out << "================= Order Book Snapshot ===================\n";
            out << "  BID Size | BID Price || Level || ASK Price | ASK Size  \n";
            out << "---------------------------------------------------------\n";
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
                if (++level > config.getOrderBookLevels())
                    break;
            }
            out << "---------------------------------------------------------\n";
        } else {
            uint level = 1;
            out << "================= Bid Book Snapshot ===================\n";
            out << " Level || BID Price | BID Size | BID Order (Time, Size)\n";
            out << "-------------------------------------------------------\n";
            while (bidIt != myBidBook.end()) {
                out << std::setw(6) << level << " || "
                    << std::fixed << std::setprecision(2)
                    << std::setw(9) << bidIt->first << " | ";
                uint32_t bidSize = 0;
                for (const auto& order : bidIt->second)
                    bidSize += order->getQuantity();
                out << std::setw(8) << bidSize << " | ";
                for (const auto& order : bidIt->second) {
                    out << "(" << order->getTimestamp() << ", " << order->getQuantity() << ") ";
                }
                out << "\n";
                ++bidIt;
                if (++level > config.getOrderBookLevels())
                    break;
            }
            out << "-------------------------------------------------------\n";
            level = 1;
            out << "================= Ask Book Snapshot ===================\n";
            out << " Level || ASK Price | ASK Size | ASK Order (Time, Size)\n";
            out << "-------------------------------------------------------\n";
            while (askIt != myAskBook.end()) {
                out << std::setw(6) << level << " || "
                    << std::fixed << std::setprecision(2)
                    << std::setw(9) << askIt->first << " | ";
                uint32_t askSize = 0;
                for (const auto& order : askIt->second)
                    askSize += order->getQuantity();
                out << std::setw(8) << askSize << " | ";
                for (const auto& order : askIt->second) {
                    out << "(" << order->getTimestamp() << ", " << order->getQuantity() << ") ";
                }
                out << "\n";
                ++askIt;
                if (++level > config.getOrderBookLevels())
                    break;
            }
            out << "-------------------------------------------------------\n";
        }
    }

    if (config.isShowTradeLog()) {
        out << "====================== Trade Log ========================\n";
        out << "   Id   |  Timestamp  |    Price    |   Size   |   Side  \n";
        out << "---------------------------------------------------------\n";
        auto tradeIt = myTradeLog.end();
        uint level = 1;
        while (tradeIt != myTradeLog.begin()) {
            const auto& trade = *--tradeIt;
            out << std::setw(6) << trade->getId() << "  | "
                << std::setw(10) << trade->getTimestamp() << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << trade->getPrice() << "  | "
                << std::setw(7) << trade->getQuantity() << "  | "
                << std::setw(6) << (trade->getIsBuyInitiated() ? "BUY" : "SELL") << "  \n";
            if (++level > config.getTradeLogLevels())
                break;
        }
        out << "---------------------------------------------------------\n";
    }

    if (config.isShowMarketQueue()) {
        out << "=============== Market Queue ==============\n";
        out << "   Id   |  Timestamp  |   Size   |   Side  \n";
        out << "-------------------------------------------\n";
        auto marketIt = myMarketQueue.end();
        uint level = 1;
        while (marketIt != myMarketQueue.begin()) {
            const auto& order = *--marketIt;
            out << std::setw(6) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::setw(7) << order->getQuantity() << "  | "
                << std::setw(6) << order->getSide() << "  \n";
            if (++level > config.getMarketQueueLevels())
                break;
        }
        out << "-------------------------------------------\n";
    }

    if (config.isShowRemovedLimitOrderLog()) {
        out << "========================= Removed Limit Orders =======================\n";
        out << "   Id   |  Timestamp  |    Price    |   Size   |   Side   |   State   \n";
        out << "----------------------------------------------------------------------\n";
        auto removedIt = myRemovedLimitOrderLog.end();
        uint level = 1;
        while (removedIt != myRemovedLimitOrderLog.begin()) {
            const auto& order = *--removedIt;
            out << std::setw(6) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << order->getPrice() << "  | "
                << std::setw(7) << order->getQuantity() << "  | "
                << std::setw(7) << order->getSide() << "  | "
                << std::setw(8) << order->getOrderState() << "  \n";
            if (++level > config.getRemovedLimitOrderLogLevels())
                break;
        }
        out << "----------------------------------------------------------------------\n";
    }

    if (config.isShowOrderLookup()) {
        // TODO
    }

    return out;
}

void MatchingEngineBase::init() {
    // TODO
}

void MatchingEngineBase::reset() {
    myBidBook.clear();
    myAskBook.clear();
    myBidBookSize.clear();
    myAskBookSize.clear();
    myMarketQueue.clear();
    myTradeLog.clear();
    myRemovedLimitOrderLog.clear();
    myLimitOrderLookup.clear();
    IMatchingEngine::reset();
}

const std::string MatchingEngineBase::getAsJson() const {
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
    OrderIndex& limitOrderLookup,
    Counter::IdHandlerBase& tradeIdHandler) {
    const uint64_t orderId = order->getId();
    const bool isIncomingOrderBuy = order->isBuy();
    auto queueIt = matchQueue.begin();
    while (unfilledQuantity && queueIt != matchQueue.end()) {
        auto& matchOrder = *queueIt;
        std::cout << "DEBUG: Matching order: " << *matchOrder << std::endl;
        const uint32_t matchQuantity = matchOrder->getQuantity();
        uint32_t filledQuantity = false;
        if (matchQuantity <= unfilledQuantity) {
            filledQuantity = matchQuantity;
            unfilledQuantity -= matchQuantity;
            matchSizeTotal -= matchQuantity;
            matchOrder->setQuantity(0);
            matchOrder->setOrderState(Market::OrderState::FILLED);
            removedLimitOrderLog.push_back(matchOrder);
            limitOrderLookup.erase(matchOrder->getId());
            queueIt = matchQueue.erase(queueIt);
        } else {
            filledQuantity = unfilledQuantity;
            matchOrder->setQuantity(matchQuantity - unfilledQuantity);
            unfilledQuantity = 0;
        }
        if (isIncomingOrderBuy)
            tradeLog.push_back(std::make_shared<Market::TradeBase>(tradeIdHandler.generateId(), order->getTimestamp(), order->getId(), matchOrder->getId(), filledQuantity, matchOrder->getPrice(), true, true, true));
        else
            tradeLog.push_back(std::make_shared<Market::TradeBase>(tradeIdHandler.generateId(), order->getTimestamp(), matchOrder->getId(), order->getId(), filledQuantity ? matchQuantity : unfilledQuantity, matchOrder->getPrice(), true, true, false));
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
    Counter::IdHandlerBase& tradeIdHandler = getTradeIdHandler();
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty() && price >= askBook.begin()->first) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup, tradeIdHandler);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        placeLimitOrderToLimitOrderBook(order, unfilledQuantity, bidBookSize[price], bidBook[price], limitOrderLookup);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty() && price <= bidBook.begin()->first) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup, tradeIdHandler);
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
    Counter::IdHandlerBase& tradeIdHandler = getTradeIdHandler();
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty()) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup, tradeIdHandler);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        placeMarketOrderToMarketOrderQueue(order, unfilledQuantity, marketQueue);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty()) {
            fillOrderByMatchingLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second, tradeLog, removedLimitOrderLog, limitOrderLookup, tradeIdHandler);
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
