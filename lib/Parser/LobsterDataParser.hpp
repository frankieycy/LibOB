#ifndef LOBSTER_DATA_PARSER_HPP
#define LOBSTER_DATA_PARSER_HPP
#include "Utils/Utils.hpp"

namespace Parser {
using namespace Utils;

class LobsterDataParser {
public:
    enum class MessageType {
        ORDER_ADD,
        ORDER_CANCEL,
        ORDER_DELETE,
        ORDER_EXECUTE_VISIBLE,
        ORDER_EXECUTE_HIDDEN,
        TRADING_HALT,
        NULL_MESSAGE_TYPE
    };

    struct OrderBookMessage {
        uint64_t timestamp;
        MessageType messageType;
        uint64_t orderId;
        uint32_t quantity;
        uint32_t price;
        bool isBuy;

        OrderBookMessage() = delete;
        OrderBookMessage(const uint64_t timestamp, const MessageType messageType, const uint64_t orderId,
                         const uint32_t quantity, const uint32_t price, const bool isBuy) :
            timestamp(timestamp), messageType(messageType), orderId(orderId), quantity(quantity), price(price), isBuy(isBuy) {}
        std::string getAsCsv() const;
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
        std::string getAsCsv() const;
    };

    bool isDebugMode() const { return myDebugMode; }
    void setDebugMode(const bool debugMode) { myDebugMode = debugMode; }

    void addOrderBookMessageAndSnapshot(
        const std::shared_ptr<const OrderBookMessage>& message,
        const std::shared_ptr<const OrderBookSnapshot>& snapshot) {
        myOrderBookMessagesCollector.addSample(message);
        myOrderBookSnapshotsCollector.addSample(snapshot);
    }

private:
    bool myDebugMode = false;
    Statistics::TimeSeriesCollector<OrderBookMessage> myOrderBookMessagesCollector;
    Statistics::TimeSeriesCollector<OrderBookSnapshot> myOrderBookSnapshotsCollector;
};
}

#endif
