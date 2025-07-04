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

IMatchingEngine::IMatchingEngine(const IMatchingEngine& matchingEngine) :
    mySymbol(matchingEngine.mySymbol),
    myExchangeId(matchingEngine.myExchangeId),
    myOrderMatchingStrategy(matchingEngine.myOrderMatchingStrategy),
    myOrderBookDisplayConfig(matchingEngine.myOrderBookDisplayConfig),
    myTradeIdHandler(matchingEngine.myTradeIdHandler),
    myReportIdHandler(matchingEngine.myReportIdHandler),
    // ensure that a new instance of the world clock and logger is created
    myWorldClock(matchingEngine.myWorldClock ? matchingEngine.myWorldClock->clone() : std::make_shared<Utils::Counter::TimestampHandlerBase>()),
    myLogger(std::make_shared<Utils::Logger::LoggerBase>()) {
    setDebugMode(matchingEngine.myDebugMode);
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

MatchingEngineBase::MatchingEngineBase(const MatchingEngineBase& matchingEngine) :
    IMatchingEngine(matchingEngine),
    myBidBook(matchingEngine.myBidBook),
    myAskBook(matchingEngine.myAskBook),
    myBidBookSize(matchingEngine.myBidBookSize),
    myAskBookSize(matchingEngine.myAskBookSize),
    myMarketQueue(matchingEngine.myMarketQueue),
    myTradeLog(matchingEngine.myTradeLog),
    myOrderEventLog(matchingEngine.myOrderEventLog),
    myOrderProcessingReportLog(matchingEngine.myOrderProcessingReportLog),
    myITCHMessageLog(matchingEngine.myITCHMessageLog),
    myRemovedLimitOrderLog(matchingEngine.myRemovedLimitOrderLog) {
    *getLogger() << Logger::LogLevel::INFO << "[MatchingEngineBase] Copy constructor leaves out the order processing callback - re-establish it if needed.";
    // construct myLimitOrderLookup by traversing each individual order in the bid and ask books
    for (auto& priceQueuePair : myBidBook) {
        LimitQueue& limitQueue = priceQueuePair.second;
        auto it = limitQueue.begin();
        while (it != limitQueue.end()) {
            const auto& order = *it;
            myLimitOrderLookup[order->getId()] = {&limitQueue, it};
            ++it;
        }
    }
    for (auto& priceQueuePair : myAskBook) {
        LimitQueue& limitQueue = priceQueuePair.second;
        auto it = limitQueue.begin();
        while (it != limitQueue.end()) {
            const auto& order = *it;
            myLimitOrderLookup[order->getId()] = {&limitQueue, it};
            ++it;
        }
    }
    init();
}

MatchingEngineBase::MatchingEngineBase() :
    IMatchingEngine() {
    init();
}

MatchingEngineBase::MatchingEngineBase(const OrderEventLog& orderEventLog) :
    IMatchingEngine() {
    build(orderEventLog);
    init();
}

MatchingEngineBase::MatchingEngineBase(const OrderProcessingReportLog& orderProcessingReportLog) :
    IMatchingEngine() {
    build(orderProcessingReportLog);
    init();
}

DescOrderBook MatchingEngineBase::getBidBook(const size_t numLevels) const {
    DescOrderBook bidBook;
    if (numLevels == 0 || myBidBook.empty())
        return bidBook;
    size_t levels = 0;
    for (const auto& priceQueuePair : myBidBook) {
        if (levels >= numLevels)
            break;
        bidBook.insert(priceQueuePair);
        ++levels;
    }
    return bidBook;
}

AscOrderBook MatchingEngineBase::getAskBook(const size_t numLevels) const {
    AscOrderBook askBook;
    if (numLevels == 0 || myAskBook.empty())
        return askBook;
    size_t levels = 0;
    for (const auto& priceQueuePair : myAskBook) {
        if (levels >= numLevels)
            break;
        askBook.insert(priceQueuePair);
        ++levels;
    }
    return askBook;
}

DescOrderBookSize MatchingEngineBase::getBidBookSize(const size_t numLevels) const {
    DescOrderBookSize bidBookSize;
    if (numLevels == 0 || myBidBookSize.empty())
        return bidBookSize;
    size_t levels = 0;
    for (const auto& priceSizePair : myBidBookSize) {
        if (levels >= numLevels)
            break;
        bidBookSize.insert(priceSizePair);
        ++levels;
    }
    return bidBookSize;
}

AscOrderBookSize MatchingEngineBase::getAskBookSize(const size_t numLevels) const {
    AscOrderBookSize askBookSize;
    if (numLevels == 0 || myAskBookSize.empty())
        return askBookSize;
    size_t levels = 0;
    for (const auto& priceSizePair : myAskBookSize) {
        if (levels >= numLevels)
            break;
        askBookSize.insert(priceSizePair);
        ++levels;
    }
    return askBookSize;
}

std::vector<DescOrderBookSize::const_iterator> MatchingEngineBase::getBidBookSizeIterators(const size_t numLevels) const {
    std::vector<DescOrderBookSize::const_iterator> iters;
    iters.reserve(numLevels);
    size_t count = 0;
    for (auto it = myBidBookSize.begin(); it != myBidBookSize.end() && count < numLevels; ++it, ++count)
        iters.push_back(it);
    return iters;
}

std::vector<AscOrderBookSize::const_iterator> MatchingEngineBase::getAskBookSizeIterators(const size_t numLevels) const {
    std::vector<AscOrderBookSize::const_iterator> iters;
    iters.reserve(numLevels);
    size_t count = 0;
    for (auto it = myAskBookSize.begin(); it != myAskBookSize.end() && count < numLevels; ++it, ++count)
        iters.push_back(it);
    return iters;
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

double MatchingEngineBase::getBidPriceAtLevel(const size_t level) const {
    if (myBidBookSize.empty() || level >= myBidBookSize.size()) {
        *getLogger() << Logger::LogLevel::WARNING << "[MatchingEngineBase::getBidPriceAtLevel] Invalid level: " << level;
        return Consts::NAN_DOUBLE;
    }
    auto it = myBidBookSize.begin();
    std::advance(it, level);
    return it->first;
}

double MatchingEngineBase::getAskPriceAtLevel(const size_t level) const {
    if (myAskBookSize.empty() || level >= myAskBookSize.size()) {
        *getLogger() << Logger::LogLevel::WARNING << "[MatchingEngineBase::getAskPriceAtLevel] Invalid level: " << level;
        return Consts::NAN_DOUBLE;
    }
    auto it = myAskBookSize.begin();
    std::advance(it, level);
    return it->first;
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
    if (!order) {
        *getLogger() << Logger::LogLevel::WARNING << "[MatchingEngineBase::process] Order is null.";
        return;
    }
    order->submit(*this); // relegate the order processing to OrderBase since it knows about the order type
}

void MatchingEngineBase::process(const std::shared_ptr<const Market::OrderEventBase>& event) {
    // the hardcore order processing engine that interacts with external order event streams
    if (!event) {
        *getLogger() << Logger::LogLevel::WARNING << "[MatchingEngineBase::process] Order event is null.";
        return;
    }
    myOrderEventLog.push_back(event);
    if (event->isSubmit()) {
        process(event->getOrder());
        return;
    }
    // limit order events handling
    const auto& it = myLimitOrderLookup.find(event->getOrderId());
    if (it != myLimitOrderLookup.end()) {
        auto& queueOrderPair = it->second;
        auto& queue = queueOrderPair.first;
        auto& orderIt = queueOrderPair.second;
        auto order = *orderIt; // owns the order so that erasal in LimitQueue keeps the order existent
        const Market::Side side = order->getSide(); // side must remain invariant for all order events
        const uint64_t oldId = order->getId();
        const double oldPrice = order->getPrice();
        const uint32_t oldQuantity = order->getQuantity();
        order->executeOrderEvent(*event);
        order->setTimestamp(clockTick());
        if (order->isAlive()) { // order event is not a cancellation
            const uint64_t newId = order->getId();
            const double newPrice = order->getPrice();
            const uint32_t newQuantity = order->getQuantity();
            queue->erase(orderIt);
            LimitQueue& newQueue = order->isBuy() ? myBidBook[newPrice] : myAskBook[newPrice];
            newQueue.push_back(order);
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
            if (newId != oldId) { // order gets replaced
                myLimitOrderLookup.erase(it);
                myLimitOrderLookup[newId] = {&newQueue, std::prev(newQueue.end())};
                logOrderProcessingReport(std::make_shared<OrderCancelAndReplaceReport>(generateReportId(), clockTick(), oldId, side, Market::OrderType::LIMIT, newId, newQuantity, newPrice, OrderProcessingStatus::SUCCESS));
            } else { // order price/quantity gets modified
                it->second = {&newQueue, std::prev(newQueue.end())};
                if (newPrice != oldPrice)
                    logOrderProcessingReport(std::make_shared<OrderModifyPriceReport>(generateReportId(), clockTick(), oldId, side, oldQuantity, newPrice, OrderProcessingStatus::SUCCESS));
                if (newQuantity != oldQuantity)
                    logOrderProcessingReport(std::make_shared<OrderModifyQuantityReport>(generateReportId(), clockTick(), oldId, side, oldPrice, newQuantity, OrderProcessingStatus::SUCCESS));
            }
        } else { // order gets cancelled
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
            logOrderProcessingReport(std::make_shared<OrderCancelReport>(generateReportId(), clockTick(), oldId, side, Market::OrderType::LIMIT, oldQuantity, oldPrice, OrderProcessingStatus::SUCCESS));
        }
    }
    // TODO: market order events handling
}

void MatchingEngineBase::build(const OrderEventLog& orderEventLog) {
    for (const auto& event : orderEventLog) {
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Processing order event: " << *event;
        if (event)
            process(event);
    }
}

void MatchingEngineBase::build(const OrderProcessingReportLog& orderProcessingReportLog) {
    for (const auto& report : orderProcessingReportLog) {
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Processing order report: " << *report;
        if (report)
            process(report->makeEvent());
    }
}

void MatchingEngineBase::build(const ITCHMessageLog& itchMessageLog) {
    for (const auto& message : itchMessageLog) {
        if (isDebugMode())
            *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineBase] Processing ITCH message: " << *message;
        if (message)
            process(message->makeEvent());
    }
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
        out << "    Id    |  Timestamp  |   Side   |    Price    |   Size   |   State   \n";
        out << "------------------------------------------------------------------------\n";
        uint level = 1;
        for (const auto& orderPair : myLimitOrderLookup) {
            const auto& order = *orderPair.second.second;
            out << std::setw(8) << order->getId() << "  | "
                << std::setw(10) << order->getTimestamp() << "  | "
                << std::setw(7) << order->getSide() << "  | "
                << std::fixed << std::setprecision(2)
                << std::setw(10) << order->getPrice() << "  | "
                << std::setw(7) << order->getQuantity() << "  | "
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
    myOrderEventLog.reserve(numOrdersEstimate);
    myOrderProcessingReportLog.reserve(numOrdersEstimate);
    myITCHMessageLog.reserve(numOrdersEstimate);
    myRemovedLimitOrderLog.reserve(numOrdersEstimate);
    myLimitOrderLookup.reserve(numOrdersEstimate);
}

void MatchingEngineBase::stateConsistencyCheck() const {
    const bool isFIFOBook = getOrderMatchingStrategy() == Exchange::OrderMatchingStrategy::FIFO;
    std::set<uint64_t> orderIds;
    if (myBidBook.size() != myBidBookSize.size()) // checks for consistency in the number of price levels
        Error::LIB_THROW("[MatchingEngineBase::init] Bid book size mismatch: " + std::to_string(myBidBook.size()) + " vs " + std::to_string(myBidBookSize.size()) + ".");
    if (myAskBook.size() != myAskBookSize.size())
        Error::LIB_THROW("[MatchingEngineBase::init] Ask book size mismatch: " + std::to_string(myAskBook.size()) + " vs " + std::to_string(myAskBookSize.size()) + ".");
    for (const auto& priceQueuePair : myBidBook) {
        const PriceLevel priceLevel = priceQueuePair.first;
        const LimitQueue& limitQueue = priceQueuePair.second;
        const uint32_t queueSize = myBidBookSize.at(priceLevel);
        auto it = limitQueue.begin();
        uint64_t priorOrderTimestamp = 0;
        uint32_t cumQueueSize = 0;
        while (it != limitQueue.end()) {
            const auto& order = *it;
            if (!order) // checks for null order
                Error::LIB_THROW("[MatchingEngineBase::init] Null order found in bid book.");
            const auto orderId = order->getId();
            const auto orderTimestamp = order->getTimestamp();
            const auto orderQuantity = order->getQuantity();
            const auto& orderIndex = myLimitOrderLookup.at(orderId);
            if (orderIds.find(orderId) != orderIds.end()) // checks for duplicate order id
                Error::LIB_THROW("[MatchingEngineBase::init] Duplicate order id found in bid book: " + std::to_string(orderId) + ".");
            if (isFIFOBook && order->getTimestamp() < priorOrderTimestamp) // checks for order timestamp sorting
                Error::LIB_THROW("[MatchingEngineBase::init] Orders in bid book are not sorted by timestamp: " + std::to_string(orderId) + ".");
            if (orderIndex.first != &limitQueue || orderIndex.second != it) // checks for order index consistency
                Error::LIB_THROW("[MatchingEngineBase::init] Order lookup index mismatch for order id: " + std::to_string(orderId) + ".");
            orderIds.insert(orderId);
            priorOrderTimestamp = orderTimestamp;
            cumQueueSize += orderQuantity;
            ++it;
        }
        if (cumQueueSize != queueSize) // checks for order queue size consistency
            Error::LIB_THROW("[MatchingEngineBase::init] Bid book size mismatch at price level " + std::to_string(priceLevel) + ": expected " + std::to_string(queueSize) + ", got " + std::to_string(cumQueueSize) + ".");
    }
    for (const auto& priceQueuePair : myAskBook) {
        const PriceLevel priceLevel = priceQueuePair.first;
        const LimitQueue& limitQueue = priceQueuePair.second;
        const uint32_t queueSize = myAskBookSize.at(priceLevel);
        auto it = limitQueue.begin();
        uint64_t priorOrderTimestamp = 0;
        uint32_t cumQueueSize = 0;
        while (it != limitQueue.end()) {
            const auto& order = *it;
            if (!order) // checks for null order
                Error::LIB_THROW("[MatchingEngineBase::init] Null order found in ask book.");
            const auto orderId = order->getId();
            const auto orderTimestamp = order->getTimestamp();
            const auto orderQuantity = order->getQuantity();
            const auto& orderIndex = myLimitOrderLookup.at(orderId);
            if (orderIds.find(orderId) != orderIds.end()) // checks for duplicate order id
                Error::LIB_THROW("[MatchingEngineBase::init] Duplicate order id found in ask book: " + std::to_string(orderId) + ".");
            if (isFIFOBook && order->getTimestamp() < priorOrderTimestamp) // checks for order timestamp sorting
                Error::LIB_THROW("[MatchingEngineBase::init] Orders in ask book are not sorted by timestamp: " + std::to_string(orderId) + ".");
            if (orderIndex.first != &limitQueue || orderIndex.second != it) // checks for order index consistency
                Error::LIB_THROW("[MatchingEngineBase::init] Order lookup index mismatch for order id: " + std::to_string(orderId) + ".");
            orderIds.insert(orderId);
            priorOrderTimestamp = orderTimestamp;
            cumQueueSize += orderQuantity;
            ++it;
        }
        if (cumQueueSize != queueSize) // checks for order queue size consistency
            Error::LIB_THROW("[MatchingEngineBase::init] Ask book size mismatch at price level " + std::to_string(priceLevel) + ": expected " + std::to_string(queueSize) + ", got " + std::to_string(cumQueueSize) + ".");
    }
    for (const auto& order : myRemovedLimitOrderLog) {
        if (!order) // checks for null order
            Error::LIB_THROW("[MatchingEngineBase::init] Null order found in removed limit order log.");
        const auto orderId = order->getId();
        if (orderIds.find(orderId) != orderIds.end()) // checks for removed limit order id presence in the order book
            Error::LIB_THROW("[MatchingEngineBase::init] Removed limit order id found in order book: " + std::to_string(orderId) + ".");
    }
}

void MatchingEngineBase::init() {
    stateConsistencyCheck();
}

void MatchingEngineBase::reset() {
    myBidBook.clear();
    myAskBook.clear();
    myBidBookSize.clear();
    myAskBookSize.clear();
    myMarketQueue.clear();
    myTradeLog.clear();
    myOrderEventLog.clear();
    myOrderProcessingReportLog.clear();
    myITCHMessageLog.clear();
    myRemovedLimitOrderLog.clear();
    myLimitOrderLookup.clear();
    IMatchingEngine::reset();
}

std::string MatchingEngineBase::getAsJson() const {
    // minimal Json representation of the matching engine state
    std::ostringstream oss;
    oss << "{"
    "\"Symbol\":\""        << getSymbol()     << "\","
    "\"ExchangeId\":\""    << getExchangeId() << "\","
    "\"OrderEventLog\":[";
    // TODO: use operator<< override for vector of pointers - implement in Utils.hpp
    for (size_t i = 0; i < myOrderEventLog.size(); ++i) {
        const auto& event = myOrderEventLog[i];
        if (event)
            oss << *event;
        if (i < myOrderEventLog.size() - 1)
            oss << ",";
    }
    oss << "]}";
    return oss.str();
}

void MatchingEngineBase::executeAgainstQueuedMarketOrders(
    const std::shared_ptr<Market::LimitOrder>& order,
    uint32_t& unfilledQuantity,
    MarketQueue& marketQueue) {
    const uint64_t orderId = order->getId();
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
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), orderId, Market::OrderType::LIMIT, order->getSide(), marketOrderId, trade->getId(), trade->getQuantity(), trade->getPrice(), true, takerOrderExecType, OrderProcessingStatus::SUCCESS)); // incoming maker order (limit order)
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), marketOrderId, Market::OrderType::MARKET, marketOrder->getSide(), orderId, trade->getId(), trade->getQuantity(), trade->getPrice(), false, makerOrderExecType, OrderProcessingStatus::SUCCESS)); // resting taker order
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
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), orderId, order->getOrderType(), order->getSide(), matchOrderId, trade->getId(), trade->getQuantity(), trade->getPrice(), false, takerOrderExecType, OrderProcessingStatus::SUCCESS)); // incoming taker order
        logOrderProcessingReport(std::make_shared<OrderExecutionReport>(generateReportId(), clockTick(), matchOrderId, Market::OrderType::LIMIT, matchOrder->getSide(), orderId, trade->getId(), trade->getQuantity(), trade->getPrice(), true, makerOrderExecType, OrderProcessingStatus::SUCCESS)); // resting maker order (limit order)
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
    const auto& message = report->makeITCHMessage();
    myOrderProcessingReportLog.push_back(report);
    myITCHMessageLog.push_back(message);
    for (const auto& callback : myOrderProcessingCallbacks)
        if (callback && *callback) (*callback)(report);
    for (const auto& callback : myITCHMessageCallbacks)
        if (callback && *callback) (*callback)(message);
}

MatchingEngineFIFO::MatchingEngineFIFO() :
    MatchingEngineBase() {
    init();
}

MatchingEngineFIFO::MatchingEngineFIFO(const MatchingEngineFIFO& matchingEngine) :
    MatchingEngineBase(matchingEngine) {
    init();
}

void MatchingEngineFIFO::addToLimitOrderBook(std::shared_ptr<Market::LimitOrder> order) {
    if (isDebugMode())
        *getLogger() << Logger::LogLevel::DEBUG << "[MatchingEngineFIFO] Add to limit order book: " << *order;
    if (!order->isAlive())
        return;
    const Market::Side side = order->getSide();
    const PriceLevel price = order->getPrice();
    uint32_t unfilledQuantity = order->getQuantity();
    DescOrderBook& bidBook = accessBidBook();
    DescOrderBookSize& bidBookSize = accessBidBookSize();
    AscOrderBook& askBook = accessAskBook();
    AscOrderBookSize& askBookSize = accessAskBookSize();
    MarketQueue& marketQueue = accessMarketQueue();
    LimitQueue dummyQueue; // avoids the creation of a new queue if the entire order is filled
    uint32_t dummySize = 0;
    logOrderProcessingReport(std::make_shared<LimitOrderSubmitReport>(generateReportId(), clockTick(), order->getId(), side, order->copy(), OrderProcessingStatus::SUCCESS));
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
    DescOrderBook& bidBook = accessBidBook();
    DescOrderBookSize& bidBookSize = accessBidBookSize();
    AscOrderBook& askBook = accessAskBook();
    AscOrderBookSize& askBookSize = accessAskBookSize();
    MarketQueue& marketQueue = accessMarketQueue();
    logOrderProcessingReport(std::make_shared<MarketOrderSubmitReport>(generateReportId(), clockTick(), order->getId(), side, order->copy(), OrderProcessingStatus::SUCCESS));
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
