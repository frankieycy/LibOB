#ifndef ITCH_ENCODER_HPP
#define ITCH_ENCODER_HPP
#include "Utils/Utils.hpp"

namespace Exchange {
struct OrderExecutionReport;
struct LimitOrderSubmitReport;
struct MarketOrderSubmitReport;
struct OrderModifyPriceReport;
struct OrderModifyQuantityReport;
struct OrderCancelReport;
struct OrderCancelAndReplaceReport;

struct ITCHEncoder {
    static constexpr uint64_t DEFAULT_AGENT_ID = 0;
    static constexpr char DEFAULT_SYMBOL[8] = "0000000";

    enum class EventCode { MARKET_OPEN, MARKET_CLOSE };
    enum class MessageType {
        SYSTEM,
        ORDER_ADD,
        ORDER_ADD_WITH_MPID,
        ORDER_EXECUTE,
        ORDER_EXECUTE_WITH_PRICE,
        ORDER_DELETE,
        ORDER_CANCEL,
        ORDER_REPLACE,
        TRADE,
        CROSS_TRADE,
        BROKEN_TRADE
    };

    /* An inheritance hierachy of ITCH messages - the data types do not necessarily follow the NASDAQ ITCH protocol
        but each class exposes an interface to cast the structure into a NASDAQ ITCH-compatible format */
    struct ITCHMessage {
        ITCHMessage() = delete;
        ITCHMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId) :
            messageId(messageId), timestamp(timestamp), agentId(agentId) {}
        virtual ~ITCHMessage() = default;
        virtual std::string toString() const = 0;
        MessageType messageType;
        uint64_t messageId;
        uint64_t timestamp;
        uint64_t agentId; // aka. stock locate
    };

    struct ITCHSystemMessage : public ITCHMessage {
        ITCHSystemMessage() = delete;
        ITCHSystemMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const EventCode eventCode) :
            ITCHMessage(messageId, timestamp, agentId), eventCode(eventCode) {
            messageType = ourType;
        }
        virtual ~ITCHSystemMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::SYSTEM;
        static const std::string ourDescription;
        EventCode eventCode;
    };

    struct ITCHOrderAddMessage : public ITCHMessage {
        ITCHOrderAddMessage() = delete;
        ITCHOrderAddMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const char symbol[8],
                            const uint64_t orderId, const bool isBuy, const uint32_t quantity, const uint32_t price) :
            ITCHMessage(messageId, timestamp, agentId), orderId(orderId), isBuy(isBuy), quantity(quantity), price(price) {
            messageType = ourType;
            std::copy(symbol, symbol + 8, this->symbol);
        }
        virtual ~ITCHOrderAddMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_ADD;
        static const std::string ourDescription;
        char symbol[8];
        uint64_t orderId;
        bool isBuy; // cast to B or S
        uint32_t quantity;
        uint32_t price; // double price x 10000
    };

    struct ITCHOrderAddWithMPIDMessage : public ITCHOrderAddMessage {
        ITCHOrderAddWithMPIDMessage() = delete;
        ITCHOrderAddWithMPIDMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const char symbol[8],
                                    const uint64_t orderId, const bool isBuy, const uint32_t quantity, const uint32_t price, const char mpid[4]) :
            ITCHOrderAddMessage(messageId, timestamp, agentId, symbol, orderId, isBuy, quantity, price) {
            messageType = ourType;
            std::copy(mpid, mpid + 4, this->mpid);
        }
        virtual ~ITCHOrderAddWithMPIDMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_ADD_WITH_MPID;
        static const std::string ourDescription;
        char mpid[4]; // market participant id, aka. attribution
    };

    /* Execution against a displayed limit order on the book */
    struct ITCHOrderExecuteMessage : public ITCHMessage {
        ITCHOrderExecuteMessage() = delete;
        ITCHOrderExecuteMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t orderId,
                                const uint64_t matchOrderId, const uint32_t fillQuantity) :
            ITCHMessage(messageId, timestamp, agentId), orderId(orderId), matchOrderId(matchOrderId), fillQuantity(fillQuantity) {
            messageType = ourType;
        }
        virtual ~ITCHOrderExecuteMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_EXECUTE;
        static const std::string ourDescription;
        uint64_t orderId;
        uint64_t matchOrderId;
        uint32_t fillQuantity;
    };

    struct ITCHOrderExecuteWithPriceMessage : public ITCHOrderExecuteMessage {
        ITCHOrderExecuteWithPriceMessage() = delete;
        ITCHOrderExecuteWithPriceMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t orderId,
                                         const uint64_t matchOrderId, const uint32_t fillQuantity, const uint32_t fillPrice) :
            ITCHOrderExecuteMessage(messageId, timestamp, agentId, orderId, matchOrderId, fillQuantity), fillPrice(fillPrice) {
            messageType = ourType;
        }
        virtual ~ITCHOrderExecuteWithPriceMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_EXECUTE_WITH_PRICE;
        static const std::string ourDescription;
        uint32_t fillPrice;
    };

    struct ITCHOrderDeleteMessage : public ITCHMessage {
        ITCHOrderDeleteMessage() = delete;
        ITCHOrderDeleteMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t orderId) :
            ITCHMessage(messageId, timestamp, agentId), orderId(orderId) {
            messageType = ourType;
        }
        virtual ~ITCHOrderDeleteMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_DELETE;
        static const std::string ourDescription;
        uint64_t orderId;
    };

    struct ITCHOrderCancelMessage : public ITCHOrderDeleteMessage {
        ITCHOrderCancelMessage() = delete;
        ITCHOrderCancelMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t orderId,
                               const uint32_t cancelQuantity) :
            ITCHOrderDeleteMessage(messageId, timestamp, agentId, orderId), cancelQuantity(cancelQuantity) {
            messageType = ourType;
        }
        virtual ~ITCHOrderCancelMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_CANCEL;
        static const std::string ourDescription;
        uint32_t cancelQuantity;
    };

    struct ITCHOrderReplaceMessage : public ITCHMessage {
        ITCHOrderReplaceMessage() = delete;
        ITCHOrderReplaceMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t oldOrderId,
                                const uint64_t newOrderId, const uint32_t quantity, const uint32_t price) :
            ITCHMessage(messageId, timestamp, agentId), oldOrderId(oldOrderId), newOrderId(newOrderId), quantity(quantity), price(price) {
            messageType = ourType;
        }
        virtual ~ITCHOrderReplaceMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::ORDER_REPLACE;
        static const std::string ourDescription;
        uint64_t oldOrderId;
        uint64_t newOrderId;
        uint32_t quantity;
        uint32_t price;
    };

    /* A new, non-persistent market order that executes immediately against the book */
    struct ITCHTradeMessage : public ITCHOrderExecuteWithPriceMessage {
        ITCHTradeMessage() = delete;
        ITCHTradeMessage(const uint64_t messageId, const uint64_t timestamp, const uint64_t agentId, const uint64_t orderId,
                         const uint64_t matchOrderId, const uint32_t fillQuantity, const uint32_t fillPrice) :
            ITCHOrderExecuteWithPriceMessage(messageId, timestamp, agentId, orderId, matchOrderId, fillQuantity, fillPrice) {
            messageType = ourType;
        }
        virtual ~ITCHTradeMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::TRADE;
        static const std::string ourDescription;
    };

    /* TODO: open/close crosses message */
    struct ITCHCrossTradeMessage : public ITCHMessage {
        ITCHCrossTradeMessage() = delete;
        virtual ~ITCHCrossTradeMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::CROSS_TRADE;
        static const std::string ourDescription;
    };

    /* TODO: trade break message */
    struct ITCHBrokenTradeMessage : public ITCHMessage {
        ITCHBrokenTradeMessage() = delete;
        virtual ~ITCHBrokenTradeMessage() = default;
        virtual std::string toString() const;
        static constexpr MessageType ourType = MessageType::BROKEN_TRADE;
        static const std::string ourDescription;
    };

    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderExecutionReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::LimitOrderSubmitReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::MarketOrderSubmitReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderModifyPriceReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderModifyQuantityReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderCancelReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderCancelAndReplaceReport& report);
};

std::string to_string(const ITCHEncoder::EventCode& eventCode);
std::string to_string(const ITCHEncoder::MessageType& messageType);
std::ostream& operator<<(std::ostream& out, const ITCHEncoder::EventCode& eventCode);
std::ostream& operator<<(std::ostream& out, const ITCHEncoder::MessageType& messageType);
}

#endif
