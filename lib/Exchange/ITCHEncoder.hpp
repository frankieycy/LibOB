#ifndef ITCH_ENCODER_HPP
#define ITCH_ENCODER_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

namespace Exchange {
class ITCHEncoder {
public:
    enum class MessageEncoding { S, A, F, E, C, X, D, U, P, Q, B };
    enum class MessageType {
        SYSTEM,
        ORDER_ADD,
        ORDER_ADD_WITH_MPID,
        ORDER_EXECUTE,
        ORDER_EXECUTE_WITH_PRICE,
        ORDER_CANCEL,
        ORDER_DELETE,
        ORDER_REPLACE,
        TRADE,
        CROSS_TRADE,
        BROKEN_TRADE
    };

    // TODO: ITCHMessage refactoring - implement as an inheritance hierarchy that exposes the toString and toBinary interface
    struct ITCHMessage {
        ITCHMessage() = delete;
        ITCHMessage(const char encoding, const uint16_t messageId, const uint64_t timestamp, const uint16_t agentId)
            : encoding(encoding), messageId(messageId), timestamp(timestamp), agentId(agentId) {}
        virtual ~ITCHMessage() = default;
        virtual std::string toString() const = 0;
        virtual std::vector<uint8_t> toBinary() const = 0;
        char encoding;
        uint16_t messageId;
        uint64_t timestamp;
        uint16_t agentId;
    };

    struct ITCHOrderAddMessage : public ITCHMessage {
        ITCHOrderAddMessage() = delete;
        ITCHOrderAddMessage(const uint16_t messageId, const uint64_t timestamp, const uint16_t agentId,
                            const char symbol[8], const uint64_t orderId, const char buyOrSell,
                            const uint32_t quantity, const uint32_t price)
            : ITCHMessage('A', messageId, timestamp, agentId), orderId(orderId), buyOrSell(buyOrSell),
              quantity(quantity), price(price) {
            std::copy(symbol, symbol + 8, this->symbol);
        }
        static const MessageType ourType = MessageType::ORDER_ADD;
        static const MessageEncoding ourEncoding = MessageEncoding::A;
        static const std::string ourDescription;
        char symbol[8];
        uint64_t orderId;
        char buyOrSell;
        uint32_t quantity;
        uint32_t price;
    };

    static const std::unordered_map<MessageType, std::pair<MessageEncoding, std::string>> MESSAGE_TYPE_ENCODINGS;
    // TODO: log the ITCH-encoded report in the matching engine
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderExecutionReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::LimitOrderSubmitReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::MarketOrderSubmitReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderModifyPriceReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderModifyQuantityReport& report);
    static std::shared_ptr<ITCHMessage> encodeReport(const Exchange::OrderCancelReport& report);
};
}

#endif
