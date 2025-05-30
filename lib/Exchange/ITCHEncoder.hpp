#ifndef ITCH_ENCODER_HPP
#define ITCH_ENCODER_HPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"

namespace Exchange {
class ITCHEncoder {
public:
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
