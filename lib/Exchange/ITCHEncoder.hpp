#ifndef ITCH_ENCODER_HPP
#define ITCH_ENCODER_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"

namespace Exchange {
class ITCHEncoder {
public:
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
    enum class MessageEncoding { S, A, F, E, C, X, D, U, P, Q, B };
    static const std::unordered_map<MessageType, std::pair<MessageEncoding, std::string>> MESSAGE_TYPE_ENCODINGS;
    static std::string encode(const Market::OrderSubmitEvent& event);
    static std::string encode(const Market::OrderFillEvent& event);
    static std::string encode(const Market::OrderModifyPriceEvent& event);
    static std::string encode(const Market::OrderModifyQuantityEvent& event);
    static std::string encode(const Market::OrderCancelEvent& event);
    static std::vector<uint8_t> encodeBinary(const Market::OrderSubmitEvent& event);
    static std::vector<uint8_t> encodeBinary(const Market::OrderFillEvent& event);
    static std::vector<uint8_t> encodeBinary(const Market::OrderModifyPriceEvent& event);
    static std::vector<uint8_t> encodeBinary(const Market::OrderModifyQuantityEvent& event);
    static std::vector<uint8_t> encodeBinary(const Market::OrderCancelEvent& event);
};
}

#endif
