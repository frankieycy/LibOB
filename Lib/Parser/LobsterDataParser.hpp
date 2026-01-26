#ifndef LOBSTER_DATA_PARSER_HPP
#define LOBSTER_DATA_PARSER_HPP
#include "Utils/Utils.hpp"

namespace Exchange {
struct OrderExecutionReport;
struct LimitOrderSubmitReport;
struct LimitOrderPlacementReport;
struct MarketOrderSubmitReport;
struct OrderModifyPriceReport;
struct OrderModifyQuantityReport;
struct OrderCancelReport;
struct OrderPartialCancelReport;
struct OrderCancelAndReplaceReport;
}

namespace Parser {
class LobsterDataParser {
public:
    enum class MessageType : uint8_t {
        ORDER_ADD = 1,                // 1: Submission of a new limit order
        ORDER_CANCEL = 2,             // 2. Cancellation (Partial deletion of a limit order)
        ORDER_DELETE = 3,             // 3. Deletion (Total deletion of a limit order)
        ORDER_EXECUTE_VISIBLE = 4,    // 4. Execution of a visible limit order
        ORDER_EXECUTE_HIDDEN = 5,     // 5. Execution of a hidden limit order
        TRADING_HALT = 7,             // 7. Trading halt indicator
        NONE = 0
    };

    struct OrderBookMessage {
        uint64_t timestamp;
        MessageType messageType;
        uint64_t orderId;
        uint32_t quantity;
        uint32_t price;
        bool isBuy;
        bool isOrderDeleteAndAdd;

        OrderBookMessage(bool isOrderDeleteAndAdd = false) :
            timestamp(0), messageType(MessageType::NONE), orderId(0), quantity(0), price(0), isBuy(true),
            isOrderDeleteAndAdd(isOrderDeleteAndAdd) {}
        OrderBookMessage(const uint64_t timestamp, const MessageType messageType, const uint64_t orderId,
                         const uint32_t quantity, const uint32_t price, const bool isBuy, const bool isOrderDeleteAndAdd = false) :
            timestamp(timestamp), messageType(messageType), orderId(orderId), quantity(quantity), price(price), isBuy(isBuy),
            isOrderDeleteAndAdd(isOrderDeleteAndAdd) {}
        OrderBookMessage(const Exchange::OrderExecutionReport& report);
        OrderBookMessage(const Exchange::LimitOrderSubmitReport& report);
        OrderBookMessage(const Exchange::LimitOrderPlacementReport& report);
        OrderBookMessage(const Exchange::MarketOrderSubmitReport& report);
        OrderBookMessage(const Exchange::OrderModifyPriceReport& report);
        OrderBookMessage(const Exchange::OrderModifyQuantityReport& report);
        OrderBookMessage(const Exchange::OrderCancelReport& report);
        OrderBookMessage(const Exchange::OrderPartialCancelReport& report);
        OrderBookMessage(const Exchange::OrderCancelAndReplaceReport& report);
        bool isValid() const { return messageType != MessageType::NONE; }
        bool toSplitIntoDeleteAndAdd() const { return isOrderDeleteAndAdd; }
        std::string getAsCsv(bool aligned = false) const;
        static std::string getHeaderCsv(bool aligned = false);
    };

    struct OrderBookSnapshot {
        std::vector<uint32_t> bidPrice;
        std::vector<uint32_t> askPrice;
        std::vector<uint32_t> bidSize;
        std::vector<uint32_t> askSize;

        OrderBookSnapshot() = default;
        OrderBookSnapshot(size_t levels) :
            bidPrice(levels, 0), askPrice(levels, 0), bidSize(levels, 0), askSize(levels, 0) {}
        OrderBookSnapshot(const std::vector<uint32_t>& bidPrice, const std::vector<uint32_t>& askPrice,
                          const std::vector<uint32_t>& bidSize, const std::vector<uint32_t>& askSize) :
            bidPrice(bidPrice), askPrice(askPrice), bidSize(bidSize), askSize(askSize) {}
        std::string getAsCsv(size_t levels, bool aligned = false) const;
        static std::string getHeaderCsv(size_t levels, bool aligned = false);
    };

    bool isDebugMode() const { return myDebugMode; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }

    void addOrderBookMessageAndSnapshot(
        const std::shared_ptr<const OrderBookMessage>& message,
        const std::shared_ptr<const OrderBookSnapshot>& snapshot) {
        myOrderBookMessagesCollector.addSample(message);
        myOrderBookSnapshotsCollector.addSample(snapshot);
    }

    std::pair<std::string, std::string> getOrderBookMessagesAndSnapshotsAsCsv(size_t snapshotLevels, bool aligned = false) const {
        std::ostringstream messagesOss;
        messagesOss << OrderBookMessage::getHeaderCsv(aligned) << "\n";
        for (const auto& message : myOrderBookMessagesCollector.getSamples())
            messagesOss << message->getAsCsv(aligned) << "\n";
        std::ostringstream snapshotsOss;
        snapshotsOss << OrderBookSnapshot::getHeaderCsv(snapshotLevels, aligned) << "\n";
        for (const auto& snapshot : myOrderBookSnapshotsCollector.getSamples())
            snapshotsOss << snapshot->getAsCsv(snapshotLevels, aligned) << "\n";
        return {messagesOss.str(), snapshotsOss.str()};
    }

private:
    bool myDebugMode = false;
    Utils::Statistics::TimeSeriesCollector<OrderBookMessage> myOrderBookMessagesCollector;
    Utils::Statistics::TimeSeriesCollector<OrderBookSnapshot> myOrderBookSnapshotsCollector;
};
}

#endif
