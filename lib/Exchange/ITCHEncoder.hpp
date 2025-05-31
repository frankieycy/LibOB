#ifndef ITCH_ENCODER_HPP
#define ITCH_ENCODER_HPP
#include "Utils/Utils.hpp"
#include "Exchange/MatchingEngineUtils.hpp"

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
    static std::string encode(const Exchange::OrderExecutionReport& report);
    static std::string encode(const Exchange::LimitOrderSubmitReport& report);
    static std::string encode(const Exchange::MarketOrderSubmitReport& report);
    static std::string encode(const Exchange::OrderModifyPriceReport& report);
    static std::string encode(const Exchange::OrderModifyQuantityReport& report);
    static std::string encode(const Exchange::OrderCancelReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::OrderExecutionReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::LimitOrderSubmitReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::MarketOrderSubmitReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::OrderModifyPriceReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::OrderModifyQuantityReport& report);
    static std::vector<uint8_t> encodeBinary(const Exchange::OrderCancelReport& report);
    static std::shared_ptr<Market::OrderEventBase> decodeStringMessage(const std::string& message);
    static std::shared_ptr<Market::OrderEventBase> decodeBinaryMessage(const std::vector<uint8_t>& message);
};
}

#endif
