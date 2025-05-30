#ifndef ITCH_ENCODER_CPP
#define ITCH_ENCODER_CPP
#include "Utils/Utils.hpp"
#include "Market/OrderEvent.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Exchange {
std::string ITCHEncoder::encode(const Market::OrderSubmitEvent& /* event */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Market::OrderFillEvent& /* event */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Market::OrderModifyPriceEvent& /* event */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Market::OrderModifyQuantityEvent& /* event */) {
    return ""; // TODO
}

std::string ITCHEncoder::encode(const Market::OrderCancelEvent& /* event */) {
    return ""; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Market::OrderSubmitEvent& /* event */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Market::OrderFillEvent& /* event */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Market::OrderModifyPriceEvent& /* event */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Market::OrderModifyQuantityEvent& /* event */) {
    return {}; // TODO
}

std::vector<uint8_t> ITCHEncoder::encodeBinary(const Market::OrderCancelEvent& /* event */) {
    return {}; // TODO
}
}

#endif
