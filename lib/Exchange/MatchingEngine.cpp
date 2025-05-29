#ifndef MATCHING_ENGINE_CPP
#define MATCHING_ENGINE_CPP
#include "Utils/Utils.hpp"
#include "Market/Order.hpp"
#include "Market/Trade.hpp"
#include "Exchange/MatchingEngineUtils.hpp"
#include "Exchange/MatchingEngine.hpp"

namespace Exchange {
using namespace Utils;

std::ostream& operator<<(std::ostream& out, const IMatchingEngine& matchingEngine) {
    return matchingEngine.orderBookSnapshot(out);
}

IMatchingEngine::IMatchingEngine(const bool debugMode) :
    myOrderBookDisplayConfig(debugMode),
    myDebugMode(debugMode) {}

void IMatchingEngine::reset() {
    mySymbol.clear();
    myExchangeId.clear();
    myTradeIdHandler.reset();
    myWorldClock->reset();
}

MatchingEngineBase::MatchingEngineBase(const OrderProcessingReportLog& orderProcessingReportLog) :
    IMatchingEngine() {
    build(orderProcessingReportLog);
    init();
}

std::pair<const PriceLevel, uint32_t> MatchingEngineBase::getBestBidPriceAndSize() const {
    if (myBidBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myBidBookSize.begin();
}

std::pair<const PriceLevel, uint32_t> MatchingEngineBase::getBestAskPriceAndSize() const {
    if (myAskBookSize.empty())
        return {Consts::NAN_DOUBLE, 0};
    return *myAskBookSize.begin();
}

std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> MatchingEngineBase::getBestBidTopOrder() const {
    if (myBidBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myBidBook.begin();
    return {it->first, it->second.front()};
}

std::pair<const PriceLevel, const std::shared_ptr<const Market::LimitOrder>> MatchingEngineBase::getBestAskTopOrder() const {
    if (myAskBook.empty())
        return {Consts::NAN_DOUBLE, nullptr};
    const auto& it = myAskBook.begin();
    return {it->first, it->second.front()};
}

double MatchingEngineBase::getBestBidPrice() const {
    if (myBidBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myBidBookSize.begin()->first;
}

double MatchingEngineBase::getBestAskPrice() const {
    if (myAskBookSize.empty())
        return Consts::NAN_DOUBLE;
    return myAskBookSize.begin()->first;
}

double MatchingEngineBase::getSpread() const {
    return getBestAskPrice() - getBestBidPrice();
}

double MatchingEngineBase::getHalfSpread() const {
    return getSpread() / 2.0;
}

double MatchingEngineBase::getMidPrice() const {
    return (getBestBidPrice() + getBestAskPrice()) / 2.0;
}

double MatchingEngineBase::getMicroPrice() const {
    return (getBestBidPrice() * getBestAskSize() + getBestAskPrice() * getBestBidSize()) / (getBestBidSize() + getBestAskSize());
}

double MatchingEngineBase::getOrderImbalance() const {
    return (getBestBidSize() - getBestAskSize()) / (getBestBidSize() + getBestAskSize());
}

double MatchingEngineBase::getLastTradePrice() const {
    return getLastTrade()->getPrice();
}

uint32_t MatchingEngineBase::getBestBidSize() const {
    if (myBidBookSize.empty())
        return 0;
    return myBidBookSize.begin()->second;
}

uint32_t MatchingEngineBase::getBestAskSize() const {
    if (myAskBookSize.empty())
        return 0;
    return myAskBookSize.begin()->second;
}

uint32_t MatchingEngineBase::getBidSize(const PriceLevel& priceLevel) const {
    const auto& it = myBidBookSize.find(priceLevel);
    return it != myBidBookSize.end() ? it->second : 0;
}

uint32_t MatchingEngineBase::getAskSize(const PriceLevel& priceLevel) const {
    const auto& it = myAskBookSize.find(priceLevel);
    return it != myAskBookSize.end() ? it->second : 0;
}

uint32_t MatchingEngineBase::getLastTradeSize() const {
    return getLastTrade()->getQuantity();
}

size_t MatchingEngineBase::getNumberOfBidPriceLevels() const {
    return myBidBook.size();
}

size_t MatchingEngineBase::getNumberOfAskPriceLevels() const {
    return myAskBook.size();
}

size_t MatchingEngineBase::getNumberOfTrades() const {
    return myTradeLog.size();
}

std::shared_ptr<const Market::TradeBase> MatchingEngineBase::getLastTrade() const {
    if (myTradeLog.empty())
        return nullptr;
    return myTradeLog.back();
}

void MatchingEngineBase::process(const std::shared_ptr<const Market::OrderBase>& order) {
    if (!order)
        Error::LIB_THROW("[MatchingEngineBase::process] Order is null.");
    order->submit(*this); // relegate the order processing to OrderBase since it knows about the order type
}

void MatchingEngineBase::process(const std::shared_ptr<const Market::OrderEventBase>& event) {
    // the hardcore order processing engine that interacts with external order event streams
    if (!event)
        Error::LIB_THROW("[MatchingEngineBase::process] Order event is null.");
    if (event->isSubmit()) {
        process(event->getOrder());
        return;
    }
    const auto& it = myLimitOrderLookup.find(event->getOrderId());
    // limit order events handling
    if (it != myLimitOrderLookup.end()) {
        auto& queueOrderPair = it->second;
        auto& queue = queueOrderPair.first;
        auto& orderIt = queueOrderPair.second;
        auto order = *orderIt; // owns the order so that erasal in LimitQueue keeps the order existent
        const double oldPrice = order->getPrice();
        const uint32_t oldQuantity = order->getQuantity();
        order->executeOrderEvent(*event);
        order->setTimestamp(clockTick());
        if (order->isAlive()) {
            const double newPrice = order->getPrice();
            const uint32_t newQuantity = order->getQuantity();
            queue->erase(orderIt);
            LimitQueue& newQueue = order->isBuy() ? myBidBook[newPrice] : myAskBook[newPrice];
            newQueue.push_back(order);
            it->second = {&newQueue, std::prev(newQueue.end())};
            if (order->isBuy()) {
                uint32_t& bidBookSizeAtNewPrice = myBidBookSize[newPrice];
                uint32_t& bidBookSizeAtOldPrice = myBidBookSize[oldPrice];
                bidBookSizeAtNewPrice += newQuantity;
                bidBookSizeAtOldPrice -= oldQuantity;
                if (bidBookSizeAtOldPrice == 0) {
                    myBidBookSize.erase(oldPrice);
                    myBidBook.erase(oldPrice);
                }
            } else {
                uint32_t& askBookSizeAtNewPrice = myAskBookSize[newPrice];
                uint32_t& askBookSizeAtOldPrice = myAskBookSize[oldPrice];
                askBookSizeAtNewPrice += newQuantity;
                askBookSizeAtOldPrice -= oldQuantity;
                if (askBookSizeAtOldPrice == 0) {
                    myAskBookSize.erase(oldPrice);
                    myAskBook.erase(oldPrice);
                }
            }
            if (newPrice != oldPrice)
                logOrderProcessingReport(std::make_shared<OrderModifyPriceReport>(generateReportId(), clockTick(), order->getId(), order->getSide(), newPrice, OrderProcessingStatus::SUCCESS));
            if (newQuantity != oldQuantity)
                logOrderProcessingReport(std::make_shared<OrderModifyQuantityReport>(generateReportId(), clockTick(), order->getId(), order->getSide(), newQuantity, OrderProcessingStatus::SUCCESS));
        } else {
            myRemovedLimitOrderLog.push_back(order);
            myLimitOrderLookup.erase(it);
            queue->erase(orderIt);
            if (order->isBuy()) {
                uint32_t& bidBookSizeAtOldPrice = myBidBookSize[oldPrice];
                bidBookSizeAtOldPrice -= oldQuantity;
                if (bidBookSizeAtOldPrice == 0) {
                    myBidBookSize.erase(oldPrice);
                    myBidBook.erase(oldPrice);
                }
            } else {
                uint32_t& askBookSizeAtOldPrice = myAskBookSize[oldPrice];
                askBookSizeAtOldPrice -= oldQuantity;
                if (askBookSizeAtOldPrice == 0) {
                    myAskBookSize.erase(oldPrice);
                    myAskBook.erase(oldPrice);
                }
            }
            logOrderProcessingReport(std::make_shared<OrderCancelReport>(generateReportId(), clockTick(), order->getId(), order->getSide(), Market::OrderType::LIMIT, OrderProcessingStatus::SUCCESS));
        }
    }
}

void MatchingEngineBase::build(const OrderProcessingReportLog& orderProcessingReportLog) {
    for (const auto& report : orderProcessingReportLog)
        process(report->makeEvent());
}

std::ostream& MatchingEngineBase::orderBookSnapshot(std::ostream& out) const {
    const OrderBookDisplayConfig& config = getOrderBookDisplayConfig();
    if (config.isShowOrderBook()) {
        auto bidIt = myBidBook.begin();
        auto askIt = myAskBook.begin();
        if (config.isAggregateOrderBook()) {
            if (config.isPrintAsciiOrderBook()) {
                uint level = 1;
                std::vector<OrderLevel> bidLevels;
                std::vector<OrderLevel> askLevels;
                while (bidIt != myBidBook.end() || askIt != myAskBook.end()) {
                    if (bidIt != myBidBook.end()) {
                        const uint32_t bidSize = myBidBookSize.at(bidIt->first);
                        bidLevels.push_back({bidIt->first, bidSize});
                        ++bidIt;
                    }
                    if (askIt != myAskBook.end()) {
                        const uint32_t askSize = myAskBookSize.at(askIt->first);
                        askLevels.push_back({askIt->first, askSize});
                        ++askIt;
                    }
                    if (++level > config.getOrderBookLevels())
                        break;
                }
                out << getOrderBookASCII(bidLevels, askLevels, config.getOrderBookBarWidth(), config.getOrderBookLevels());
            } else {
                uint level = 1;
                out << "================= Order Book Snapshot ===================\n";
                out << "  BID Size | BID Price || Level || ASK Price | ASK Size  \n";
                out << "---------------------------------------------------------\n";
                while (bidIt != myBidBook.end() || askIt != myAskBook.end()) {
                    if (bidIt != myBidBook.end()) {
                        const uint32_t bidSize = myBidBookSize.at(bidIt->first);
                        out << std::setw(9) << bidSize << "  | "
                            << std::fixed << std::setprecision(2)
                            << std::setw(8) << bidIt->first << "  || ";
                        ++bidIt;
                    } else {
                        out << "           |           || ";
                    }
                    out << std::setw(5) << level << " || ";
                    if (askIt != myAskBook.end()) {
                        const uint32_t askSize = myAskBookSize.at(askIt->first);
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
            }
        } else {
            uint level = 1;
            out << "================= Bid Book Snapshot ===================\n";
            out << " Level || BID Price | BID Size | BID Order (Id,Time|Size)\n";
            out << "-------------------------------------------------------\n";
            while (bidIt != myBidBook.end()) {
                out << std::setw(6) << level << " || "
                    << std::fixed << std::setprecision(2)
                    << std::setw(9) << bidIt->first << " | ";
                const uint32_t bidSize = myBidBookSize.at(bidIt->first);
                out << std::setw(8) << bidSize << " | ";
                for (const auto& order : bidIt->second) {
                    out << "(" << order->getId() << "," << order->getTimestamp() << "|" << order->getQuantity() << ") ";
                }
                out << "\n";
                ++bidIt;
                if (++level > config.getOrderBookLevels())
                    break;
            }
            out << "-------------------------------------------------------\n";
            level = 1;
            out << "================= Ask Book Snapshot ===================\n";
            out << " Level || ASK Price | ASK Size | ASK Order (Id,Time|Size)\n";
            out << "-------------------------------------------------------\n";
            while (askIt != myAskBook.end()) {
                out << std::setw(6) << level << " || "
                    << std::fixed << std::setprecision(2)
                    << std::setw(9) << askIt->first << " | ";
                const uint32_t askSize = myAskBookSize.at(askIt->first);
                out << std::setw(8) << askSize << " | ";
                for (const auto& order : askIt->second) {
                    out << "(" << order->getId() << "," << order->getTimestamp() << "|" << order->getQuantity() << ") ";
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
        out << "======================= Trade Log =========================\n";
        out << "    Id    |  Timestamp  |   Side   |    Price    |   Size  \n";
        out << "-----------------------------------------------------------\n";
        auto tradeIt = myTradeLog.end();
        uint level = 1;
        while (tradeIt != myTradeLog.begin()) {
            const auto& trade = *--tradeIt;
            out << std::setw(8) << trade->getId() << "  | "
                << std::setw(10) << trade->getTimestamp() << "  | "
                << std::setw(7) << (trade->getIsBuyInitiated() ? "Buy" : "Sell") << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << trade->getPrice() << "  | "
                << std::setw(6) << trade->getQuantity() << "  \n";
            if (++level > config.getTradeLogLevels())
                break;
        }
        out << "-----------------------------------------------------------\n";
    }

    if (config.isShowMarketQueue()) {
        out << "================ Market Queue ===============\n";
        out << "    Id    |  Timestamp  |   Side   |   Size  \n";
        out << "---------------------------------------------\n";
        auto marketIt = myMarketQueue.end();
        uint level = 1;
        while (marketIt != myMarketQueue.begin()) {
            const auto& order = *--marketIt;
            out << std::setw(8) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::setw(7) << order->getSide() << "  | "
                << std::setw(6) << order->getQuantity() << "  \n";
            if (++level > config.getMarketQueueLevels())
                break;
        }
        out << "---------------------------------------------\n";
    }

    if (config.isShowRemovedLimitOrderLog()) {
        out << "========================== Removed Limit Orders ========================\n";
        out << "    Id    |  Timestamp  |   Side   |    Price    |   Size   |   State   \n";
        out << "------------------------------------------------------------------------\n";
        auto removedIt = myRemovedLimitOrderLog.end();
        uint level = 1;
        while (removedIt != myRemovedLimitOrderLog.begin()) {
            const auto& order = *--removedIt;
            out << std::setw(8) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::setw(7) << order->getSide() << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << order->getPrice() << "  | "
                << std::setw(7) << order->getQuantity() << "  | "
                << std::setw(8) << order->getOrderState() << "  \n";
            if (++level > config.getRemovedLimitOrderLogLevels())
                break;
        }
        out << "------------------------------------------------------------------------\n";
    }

    if (config.isShowOrderLookup()) {
        out << "======================== Order Lookup Table ============================\n";
        out << "    Id    |  Timestamp  |    Price    |   Size   |   Side   |   State   \n";
        out << "------------------------------------------------------------------------\n";
        uint level = 1;
        for (const auto& orderPair : myLimitOrderLookup) {
            const auto& order = *orderPair.second.second;
            out << std::setw(8) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << order->getPrice() << "  | "
                << std::setw(7) << order->getQuantity() << "  | "
                << std::setw(7) << order->getSide() << "  | "
                << std::setw(8) << order->getOrderState() << "  \n";
            if (++level > config.getOrderLookupLevels())
                break;
        }
        out << "------------------------------------------------------------------------\n";
    }

    return out;
}

void MatchingEngineBase::reserve(const size_t numOrdersEstimate) {
    myTradeLog.reserve(numOrdersEstimate);
    myOrderProcessingReportLog.reserve(numOrdersEstimate);
    myRemovedLimitOrderLog.reserve(numOrdersEstimate);
    myLimitOrderLookup.reserve(numOrdersEstimate);
}

void MatchingEngineBase::init() {
    // TODO: consistency checks for order book data members requiring that
    // * the order indices are unique
    // * the orders are sorted by timestamp in the limit queues
    // * the price levels of the order books and order book sizes match
    // * the individual order sizes add up to the total size at each price level
    // * the removed limit orders are not present in the order book or market queue
    // * the limit order lookup maps exactly to the limit orders in the book
}

void MatchingEngineBase::reset() {
    myBidBook.clear();
    myAskBook.clear();
    myBidBookSize.clear();
    myAskBookSize.clear();
    myMarketQueue.clear();
    myTradeLog.clear();
    myOrderProcessingReportLog.clear();
    myRemovedLimitOrderLog.clear();
    myLimitOrderLookup.clear();
    IMatchingEngine::reset();
}

std::string MatchingEngineBase::getAsJson() const {
    // TODO
    return "";
}

void MatchingEngineBase::executeAgainstQueuedMarketOrders(
    const std::shared_ptr<Market::LimitOrder>& order,
    uint32_t& unfilledQuantity,
    MarketQueue& marketQueue) {
    auto queueIt = marketQueue.begin();
    while (unfilledQuantity && queueIt != marketQueue.end()) {
        auto marketOrder = *queueIt; // owns the order
        // continue if the queued market order is not a match
        if (marketOrder->isBuy() == order->isBuy()) {
            ++queueIt;
            continue;
        }
        const uint64_t marketOrderId = marketOrder->getId();
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Matching queued market order: " << *marketOrder;
        const uint32_t matchQuantity = marketOrder->getQuantity();
        uint32_t filledQuantity = 0;
        if (matchQuantity <= unfilledQuantity) {
            filledQuantity = matchQuantity;
            unfilledQuantity -= matchQuantity;
            marketOrder->setQuantity(0);
            marketOrder->setOrderState(Market::OrderState::FILLED);
            queueIt = marketQueue.erase(queueIt);
        } else {
            filledQuantity = unfilledQuantity;
            marketOrder->setQuantity(matchQuantity - unfilledQuantity);
            marketOrder->setOrderState(Market::OrderState::PARTIAL_FILLED);
            unfilledQuantity = 0;
        }
        // internal log of executed trades
        std::shared_ptr<const Market::TradeBase> trade;
        if (order->isBuy()) // limit buy, market sell
            trade = std::make_shared<const Market::TradeBase>(generateTradeId(), clockTick(), order->getId(), marketOrderId, filledQuantity, order->getPrice(), true, false, true);
        else // limit sell, market buy
            trade = std::make_shared<const Market::TradeBase>(generateTradeId(), clockTick(), marketOrderId, order->getId(), filledQuantity, order->getPrice(), false, true, false);
        myTradeLog.push_back(trade);
        // external callback of executed trades
        const OrderExecutionType takerOrderExecType = unfilledQuantity == 0 ? OrderExecutionType::FILLED : OrderExecutionType::PARTIAL_FILLED;
        const OrderExecutionType makerOrderExecType = marketOrder->getQuantity() == 0 ? OrderExecutionType::FILLED : OrderExecutionType::PARTIAL_FILLED;
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), order->getId(), Market::OrderType::LIMIT, order->getSide(), trade->getId(), trade->getQuantity(), trade->getPrice(), true, takerOrderExecType, OrderProcessingStatus::SUCCESS)); // incoming maker order (limit order)
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), marketOrderId, Market::OrderType::MARKET, marketOrder->getSide(), trade->getId(), trade->getQuantity(), trade->getPrice(), false, makerOrderExecType, OrderProcessingStatus::SUCCESS)); // resting taker order
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Trade executed: " << *trade;
    }
}

void MatchingEngineBase::fillOrderByMatchingTopLimitQueue(
    const std::shared_ptr<Market::OrderBase>& order,
    uint32_t& unfilledQuantity,
    uint32_t& matchSizeTotal,
    LimitQueue& matchQueue) {
    const uint64_t orderId = order->getId();
    const bool isIncomingOrderBuy = order->isBuy();
    auto queueIt = matchQueue.begin();
    while (unfilledQuantity && queueIt != matchQueue.end()) {
        auto matchOrder = *queueIt; // owns the order
        const uint64_t matchOrderId = matchOrder->getId();
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Matching order: " << *matchOrder;
        const uint32_t matchQuantity = matchOrder->getQuantity();
        uint32_t filledQuantity = 0;
        if (matchQuantity <= unfilledQuantity) {
            filledQuantity = matchQuantity;
            unfilledQuantity -= matchQuantity;
            matchSizeTotal -= matchQuantity;
            matchOrder->setQuantity(0);
            matchOrder->setOrderState(Market::OrderState::FILLED);
            myRemovedLimitOrderLog.push_back(matchOrder);
            myLimitOrderLookup.erase(matchOrderId);
            queueIt = matchQueue.erase(queueIt);
        } else {
            filledQuantity = unfilledQuantity;
            matchSizeTotal -= unfilledQuantity;
            matchOrder->setQuantity(matchQuantity - unfilledQuantity);
            matchOrder->setOrderState(Market::OrderState::PARTIAL_FILLED);
            unfilledQuantity = 0;
        }
        // internal log of executed trades
        std::shared_ptr<const Market::TradeBase> trade;
        if (isIncomingOrderBuy)
            trade = std::make_shared<const Market::TradeBase>(generateTradeId(), clockTick(), orderId, matchOrderId, filledQuantity, matchOrder->getPrice(), order->isLimitOrder(), true, true);
        else
            trade = std::make_shared<const Market::TradeBase>(generateTradeId(), clockTick(), matchOrderId, orderId, filledQuantity, matchOrder->getPrice(), true, order->isLimitOrder(), false);
        myTradeLog.push_back(trade);
        // external callback of executed trades
        const OrderExecutionType takerOrderExecType = unfilledQuantity == 0 ? OrderExecutionType::FILLED : OrderExecutionType::PARTIAL_FILLED;
        const OrderExecutionType makerOrderExecType = matchOrder->getQuantity() == 0 ? OrderExecutionType::FILLED : OrderExecutionType::PARTIAL_FILLED;
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), orderId, order->getOrderType(), order->getSide(), trade->getId(), trade->getQuantity(), trade->getPrice(), false, takerOrderExecType, OrderProcessingStatus::SUCCESS)); // incoming taker order
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), matchOrderId, Market::OrderType::LIMIT, matchOrder->getSide(), trade->getId(), trade->getQuantity(), trade->getPrice(), true, makerOrderExecType, OrderProcessingStatus::SUCCESS)); // resting maker order (limit order)
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Trade executed: " << *trade;
    }
}

void MatchingEngineBase::placeLimitOrderToLimitOrderBook(
    std::shared_ptr<Market::LimitOrder>& order,
    const uint32_t unfilledQuantity,
    uint32_t& orderSizeTotal,
    LimitQueue& limitQueue) {
    const uint32_t originalQuantity = order->getQuantity();
    if (unfilledQuantity) {
        order->setQuantity(unfilledQuantity);
        order->setTimestamp(clockTick());
        if (unfilledQuantity < originalQuantity)
            order->setOrderState(Market::OrderState::PARTIAL_FILLED);
        limitQueue.push_back(order);
        orderSizeTotal += order->getQuantity();
        myLimitOrderLookup[order->getId()] = {&limitQueue, std::prev(limitQueue.end())};
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Placed order in limit order book: " << *order;
    } else {
        order->setQuantity(0);
        order->setTimestamp(clockTick());
        order->setOrderState(Market::OrderState::FILLED);
    }
}

void MatchingEngineBase::placeMarketOrderToMarketOrderQueue(
    std::shared_ptr<Market::MarketOrder>& order,
    const uint32_t unfilledQuantity,
    MarketQueue& marketQueue) {
    const uint32_t originalQuantity = order->getQuantity();
    if (unfilledQuantity) {
        order->setQuantity(unfilledQuantity);
        order->setTimestamp(clockTick());
        if (unfilledQuantity < originalQuantity)
            order->setOrderState(Market::OrderState::PARTIAL_FILLED);
        marketQueue.push_back(order);
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Placed order in market order queue: " << *order;
    } else {
        order->setQuantity(0);
        order->setTimestamp(clockTick());
        order->setOrderState(Market::OrderState::FILLED);
    }
}

void MatchingEngineBase::logOrderProcessingReport(const std::shared_ptr<const OrderProcessingReport>& report) {
    myOrderProcessingReportLog.push_back(report);
    if (myOrderProcessingCallback)
        myOrderProcessingCallback(report);
}

void MatchingEngineFIFO::addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) {
    if (isDebugMode())
        *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineFIFO] Add to limit order book: " << *order;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    const PriceLevel price = order->getPrice();
    uint32_t unfilledQuantity = order->getQuantity();
    DescOrderBook& bidBook = getBidBook();
    DescOrderBookSize& bidBookSize = getBidBookSize();
    AscOrderBook& askBook = getAskBook();
    AscOrderBookSize& askBookSize = getAskBookSize();
    MarketQueue& marketQueue = getMarketQueue();
    OrderProcessingCallback orderProcessingCallback = getOrderProcessingCallback();
    LimitQueue dummyQueue; // avoids the creation of a new queue if the entire order is filled
    uint32_t dummySize = 0;
    if (orderProcessingCallback)
        orderProcessingCallback(std::make_shared<LimitOrderSubmitReport>(generateReportId(), clockTick(), order->getId(), side, order, OrderProcessingStatus::SUCCESS));
    executeAgainstQueuedMarketOrders(order, unfilledQuantity, marketQueue);
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty() && price >= askBook.begin()->first) {
            fillOrderByMatchingTopLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        if (unfilledQuantity)
            placeLimitOrderToLimitOrderBook(order, unfilledQuantity, bidBookSize[price], bidBook[price]);
        else
            placeLimitOrderToLimitOrderBook(order, 0, dummySize, dummyQueue);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty() && price <= bidBook.begin()->first) {
            fillOrderByMatchingTopLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second);
            if (bidBook.begin()->second.empty()) {
                bidBook.erase(bidBook.begin());
                bidBookSize.erase(bidBookSize.begin());
            }
        }
        if (unfilledQuantity)
            placeLimitOrderToLimitOrderBook(order, unfilledQuantity, askBookSize[price], askBook[price]);
        else
            placeLimitOrderToLimitOrderBook(order, 0, dummySize, dummyQueue);
    }
}

void MatchingEngineFIFO::executeMarketOrder(std::shared_ptr<Market::MarketOrder> order) {
    if (isDebugMode())
        *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineFIFO] Execute market order: " << *order;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    uint32_t unfilledQuantity = order->getQuantity();
    DescOrderBook& bidBook = getBidBook();
    DescOrderBookSize& bidBookSize = getBidBookSize();
    AscOrderBook& askBook = getAskBook();
    AscOrderBookSize& askBookSize = getAskBookSize();
    MarketQueue& marketQueue = getMarketQueue();
    OrderProcessingCallback orderProcessingCallback = getOrderProcessingCallback();
    if (orderProcessingCallback)
        orderProcessingCallback(std::make_shared<MarketOrderSubmitReport>(generateReportId(), clockTick(), order->getId(), side, order, OrderProcessingStatus::SUCCESS));
    if (side == Market::Side::BUY) {
        while (unfilledQuantity && !askBook.empty()) {
            fillOrderByMatchingTopLimitQueue(order, unfilledQuantity, askBookSize.begin()->second, askBook.begin()->second);
            if (askBook.begin()->second.empty()) {
                askBook.erase(askBook.begin());
                askBookSize.erase(askBookSize.begin());
            }
        }
        placeMarketOrderToMarketOrderQueue(order, unfilledQuantity, marketQueue);
    } else if (side == Market::Side::SELL) {
        while (unfilledQuantity && !bidBook.empty()) {
            fillOrderByMatchingTopLimitQueue(order, unfilledQuantity, bidBookSize.begin()->second, bidBook.begin()->second);
            if (bidBook.begin()->second.empty()) {
                bidBook.erase(bidBook.begin());
                bidBookSize.erase(bidBookSize.begin());
            }
        }
        placeMarketOrderToMarketOrderQueue(order, unfilledQuantity, marketQueue);
    }
}

void MatchingEngineFIFO::init() {
    MatchingEngineBase::init();
    setOrderMatchingStrategy(OrderMatchingStrategy::FIFO);
}
}

#endif
