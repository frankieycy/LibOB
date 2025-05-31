#ifndef ITCH_ENCODER_CPP
#define ITCH_ENCODER_CPP
#include "Utils/Utils.hpp"
#include "Exchange/ITCHEncoder.hpp"

namespace Exchange {
using namespace Utils;

const std::string ITCHEncoder::ITCHOrderAddMessage::ourDescription = "[A] New order entered into the book";

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderExecutionReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::LimitOrderSubmitReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::MarketOrderSubmitReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyPriceReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderModifyQuantityReport& /* report */) {
    return nullptr; // TODO
}

std::shared_ptr<ITCHEncoder::ITCHMessage> ITCHEncoder::encodeReport(const Exchange::OrderCancelReport& /* report */) {
    return nullptr; // TODO
}
}

#endif
