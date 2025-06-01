#ifndef MATCHING_ENGINE_UTILS_HPP
#define MATCHING_ENGINE_UTILS_HPP
#include "Utils/Utils.hpp"
#include "Market/Order.hpp"
#include "Market/Trade.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Market {
class OrderEventManagerBase;
}

namespace Exchange {
enum class OrderMatchingStrategy { FIFO, PRO_RATA, ICEBERG_SUPPORT, NULL_ORDER_MATCHING_STRATEGY };
enum class OrderProcessingType   { EXECUTE, SUBMIT, CANCEL, MODIFY_PRICE, MODIFY_QUANTITY, NULL_ORDER_PROCESSING_TYPE };
enum class OrderProcessingStatus { SUCCESS, FAILURE, NULL_ORDER_PROCESSING_STATUS };
enum class OrderExecutionType    { FILLED, PARTIAL_FILLED, CANCELLED, REJECTED, NULL_ORDER_EXECUTION_TYPE };

struct OrderLevel {
    double price;
    uint32_t size;
};

std::string generateBar(const uint32_t size, const uint32_t maxSize, const size_t maxWidth, const char symbol = 'o');

std::string getOrderBookASCII(
    const std::vector<OrderLevel>& bidBook,
    const std::vector<OrderLevel>& askBook,
    const size_t barWidth = 40,
    const size_t maxDepth = 10);

class OrderBookDisplayConfig {
public:
    OrderBookDisplayConfig() = default;
    OrderBookDisplayConfig(const bool debugMode);
    ~OrderBookDisplayConfig() = default;
    uint16_t getOrderBookLevels() const { return myOrderBookLevels; }
    uint16_t getMarketQueueLevels() const { return myMarketQueueLevels; }
    uint16_t getTradeLogLevels() const { return myTradeLogLevels; }
    uint16_t getRemovedLimitOrderLogLevels() const { return myRemovedLimitOrderLogLevels; }
    uint16_t getOrderLookupLevels() const { return myOrderLookupLevels; }
    size_t getOrderBookBarWidth() const { return myOrderBookBarWidth; }
    bool isAggregateOrderBook() const { return myAggregateOrderBook; }
    bool isPrintAsciiOrderBook() const { return myPrintAsciiOrderBook; }
    bool isShowOrderBook() const { return myShowOrderBook; }
    bool isShowMarketQueue() const { return myShowMarketQueue; }
    bool isShowTradeLog() const { return myShowTradeLog; }
    bool isShowRemovedLimitOrderLog() const { return myShowRemovedLimitOrderLog; }
    bool isShowOrderLookup() const { return myShowOrderLookup; }
    bool isLiveDisplay() const { return myIsLiveDisplay; }
    bool isDebugMode() const { return myDebugMode; }
    void setOrderBookLevels(const uint16_t orderBookLevels) { myOrderBookLevels = orderBookLevels; }
    void setMarketQueueLevels(const uint16_t marketQueueLevels) { myMarketQueueLevels = marketQueueLevels; }
    void setTradeLogLevels(const uint16_t tradeLogLevels) { myTradeLogLevels = tradeLogLevels; }
    void setRemovedLimitOrderLogLevels(const uint16_t removedLimitOrderLogLevels) { myRemovedLimitOrderLogLevels = removedLimitOrderLogLevels; }
    void setOrderLookupLevels(const uint16_t orderLookupLevels) { myOrderLookupLevels = orderLookupLevels; }
    void setOrderBookBarWidth(const size_t orderBookBarWidth) { myOrderBookBarWidth = orderBookBarWidth; }
    void setAggregateOrderBook(const bool aggregateOrderBook) { myAggregateOrderBook = aggregateOrderBook; }
    void setPrintAsciiOrderBook(const bool printAsciiOrderBook) { myPrintAsciiOrderBook = printAsciiOrderBook; }
    void setShowOrderBook(const bool showOrderBook) { myShowOrderBook = showOrderBook; }
    void setShowMarketQueue(const bool showMarketQueue) { myShowMarketQueue = showMarketQueue; }
    void setShowTradeLog(const bool showTradeLog) { myShowTradeLog = showTradeLog; }
    void setShowRemovedLimitOrderLog(const bool showRemovedLimitOrderLog) { myShowRemovedLimitOrderLog = showRemovedLimitOrderLog; }
    void setShowOrderLookup(const bool showOrderLookup) { myShowOrderLookup = showOrderLookup; }
    void setLiveDisplay(const bool liveDisplay) { myIsLiveDisplay = liveDisplay; }
    void setDebugMode(const bool debugMode);
private:
    uint16_t myOrderBookLevels = 5;
    uint16_t myMarketQueueLevels = 10;
    uint16_t myTradeLogLevels = 10;
    uint16_t myRemovedLimitOrderLogLevels = 10;
    uint16_t myOrderLookupLevels = 10;
    size_t myOrderBookBarWidth = 40;
    bool myAggregateOrderBook = true;
    bool myPrintAsciiOrderBook = false;
    bool myShowOrderBook = true;
    bool myShowMarketQueue = true;
    bool myShowTradeLog = true;
    bool myShowRemovedLimitOrderLog = true;
    bool myShowOrderLookup = true;
    bool myIsLiveDisplay = false;
    bool myDebugMode = false;
};

// NASDAQ ITCH-like compact order processing reports disseminated to OrderEventManager
struct OrderProcessingReport {
    OrderProcessingReport() = delete;
    OrderProcessingReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const OrderProcessingType orderProcessingType,
        const OrderProcessingStatus status,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        reportId(reportId), timestamp(timestamp), orderId(orderId), orderSide(orderSide), orderProcessingType(orderProcessingType),
        status(status), latency(latency), message(message) {}
    virtual ~OrderProcessingReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const = 0;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const = 0;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const = 0;
    virtual std::string getAsJson() const = 0;
    uint64_t reportId;
    uint64_t timestamp;
    uint64_t orderId;
    Market::Side orderSide;
    OrderProcessingType orderProcessingType;
    OrderProcessingStatus status;
    std::optional<uint64_t> latency = std::nullopt;
    std::optional<std::string> message = std::nullopt;
};

struct OrderExecutionReport : public OrderProcessingReport {
    OrderExecutionReport() = delete;
    OrderExecutionReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::OrderType orderType,
        const Market::Side orderSide,
        const uint64_t tradeId,
        const uint32_t filledQuantity,
        const double filledPrice,
        const bool isMakerOrder,
        const OrderExecutionType orderExecutionType,
        const OrderProcessingStatus status,
        const std::shared_ptr<const Market::TradeBase>& trade = nullptr,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::EXECUTE, status, latency, message),
        orderType(orderType), tradeId(tradeId), filledQuantity(filledQuantity), filledPrice(filledPrice),
        isMakerOrder(isMakerOrder), orderExecutionType(orderExecutionType), trade(trade) {}
    virtual ~OrderExecutionReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    Market::OrderType orderType;
    uint64_t tradeId;
    uint32_t filledQuantity;
    double filledPrice;
    bool isMakerOrder; // if the order is a resting maker order
    OrderExecutionType orderExecutionType;
    std::shared_ptr<const Market::TradeBase> trade = nullptr;
};

struct LimitOrderSubmitReport : public OrderProcessingReport {
    LimitOrderSubmitReport() = delete;
    LimitOrderSubmitReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const std::shared_ptr<const Market::LimitOrder>& order,
        const OrderProcessingStatus status,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::SUBMIT, status, latency, message),
        order(order) {}
    virtual ~LimitOrderSubmitReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    std::shared_ptr<const Market::LimitOrder> order = nullptr;
};

struct MarketOrderSubmitReport : public OrderProcessingReport {
    MarketOrderSubmitReport() = delete;
    MarketOrderSubmitReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const std::shared_ptr<const Market::MarketOrder>& order,
        const OrderProcessingStatus status,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::SUBMIT, status, latency, message),
        order(order) {}
    virtual ~MarketOrderSubmitReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    std::shared_ptr<const Market::MarketOrder> order = nullptr;
};

struct OrderCancelReport : public OrderProcessingReport {
    OrderCancelReport() = delete;
    OrderCancelReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const Market::OrderType orderType,
        const OrderProcessingStatus status,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::CANCEL, status, latency, message),
        orderType(orderType) {}
    virtual ~OrderCancelReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    Market::OrderType orderType;
};

struct OrderModifyPriceReport : public OrderProcessingReport {
    OrderModifyPriceReport() = delete;
    OrderModifyPriceReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const double modifiedPrice,
        const OrderProcessingStatus status,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::MODIFY_PRICE, status, latency, message),
        modifiedPrice(modifiedPrice) {}
    virtual ~OrderModifyPriceReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    double modifiedPrice;
};

struct OrderModifyQuantityReport : public OrderProcessingReport {
    OrderModifyQuantityReport() = delete;
    OrderModifyQuantityReport(
        const uint64_t reportId,
        const uint64_t timestamp,
        const uint64_t orderId,
        const Market::Side orderSide,
        const uint32_t modifiedQuantity,
        const OrderProcessingStatus status = OrderProcessingStatus::NULL_ORDER_PROCESSING_STATUS,
        const std::optional<uint64_t> latency = std::nullopt,
        const std::optional<std::string> message = std::nullopt) :
        OrderProcessingReport(reportId, timestamp, orderId, orderSide, OrderProcessingType::MODIFY_QUANTITY, status, latency, message),
        modifiedQuantity(modifiedQuantity) {}
    virtual ~OrderModifyQuantityReport() = default;
    virtual void dispatchTo(Market::OrderEventManagerBase& orderEventManager) const override;
    virtual std::shared_ptr<Market::OrderEventBase> makeEvent() const override;
    virtual std::shared_ptr<ITCHEncoder::ITCHMessage> makeITCHMessage() const override;
    virtual std::string getAsJson() const override;
    uint32_t modifiedQuantity;
};

std::string to_string(const OrderMatchingStrategy& orderMatchingStrategy);
std::string to_string(const OrderProcessingType& orderProcessingType);
std::string to_string(const OrderProcessingStatus& orderProcessingStatus);
std::string to_string(const OrderExecutionType& orderExecutionType);

std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy);
std::ostream& operator<<(std::ostream& out, const OrderProcessingType& orderProcessingType);
std::ostream& operator<<(std::ostream& out, const OrderProcessingStatus& orderProcessingStatus);
std::ostream& operator<<(std::ostream& out, const OrderExecutionType& orderExecutionType);
std::ostream& operator<<(std::ostream& out, const OrderProcessingReport& event);
}

#endif
