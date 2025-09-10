#ifndef LOBSTER_DATA_PARSER_HPP
#define LOBSTER_DATA_PARSER_HPP
#include "Utils/Utils.hpp"

namespace Parser {
class LobsterDataParser {
public:
    enum class MessageType {
        ORDER_ADD,
        ORDER_CANCEL,
        ORDER_DELETE,
        ORDER_EXECUTE_VISIBLE,
        ORDER_EXECUTE_HIDDEN,
        TRADING_HALT
    };

    struct OrderBookMessage {
        uint64_t timestamp;
        MessageType messageType;
        uint64_t orderId;
        uint32_t quantity;
        uint32_t price;
        bool isBuy;
        std::string getAsCsv() const;
    };

    struct OrderBookSnapshot {
        std::vector<uint32_t> askPrice;
        std::vector<uint32_t> askSize;
        std::vector<uint32_t> bidPrice;
        std::vector<uint32_t> bidSize;

        OrderBookSnapshot(size_t levels) :
            askPrice(levels), askSize(levels),
            bidPrice(levels), bidSize(levels) {}
        std::string getAsCsv() const;
    };
};
}

#endif
