#ifndef EXCHANGE_SIMULATOR_UTILS_CPP
#define EXCHANGE_SIMULATOR_UTILS_CPP
#include "Utils/Utils.hpp"
#include "Simulator/ExchangeSimulatorUtils.hpp"
#include "Simulator/ExchangeSimulator.hpp"

namespace Simulator {
using namespace Utils;
using Utils::operator<<;

std::ostream& operator<<(std::ostream& out, const OrderEventBase& event) { return out << event.getAsJson(); }

bool ExchangeSimulatorStopCondition::check(const IExchangeSimulator& simulator) const {
    if (maxTimestamp && simulator.getCurrentTimestamp() >= *maxTimestamp)
        return true;
    if (maxNumEvents && simulator.getCurrentNumEvents() >= *maxNumEvents)
        return true;
    return false;
}

std::string OrderEventBase::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << eventId   << ","
    "\"Timestamp\":"   << timestamp << ","
    "\"EventType\":\"" << eventType << "\"";
    oss << "}";
    return oss.str();
}

std::string LimitOrderSubmitEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << eventId   << ","
    "\"Timestamp\":"   << timestamp << ","
    "\"EventType\":\"" << eventType << "\","
    "\"Side\":\""      << side      << "\","
    "\"Quantity\":"    << quantity  << ","
    "\"Price\":"       << price;
    oss << "}";
    return oss.str();
}

std::string MarketOrderSubmitEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << eventId   << ","
    "\"Timestamp\":"   << timestamp << ","
    "\"EventType\":\"" << eventType << "\","
    "\"Side\":\""      << side      << "\","
    "\"Quantity\":"    << quantity;
    oss << "}";
    return oss.str();
}

std::string OrderCancelEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << eventId   << ","
    "\"Timestamp\":"   << timestamp << ","
    "\"EventType\":\"" << eventType << "\","
    "\"Side\":\""      << side      << "\","
    "\"Quantity\":"    << quantity  << ","
    "\"Price\":"       << price;
    oss << "}";
    return oss.str();
}

std::string OrderCancelByIdEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"     << eventId   << ","
    "\"Timestamp\":"   << timestamp << ","
    "\"EventType\":\"" << eventType << "\","
    "\"OrderId\":"     << orderId;
    oss << "}";
    return oss.str();
}

std::string OrderCancelAndReplaceEvent::getAsJson() const {
    std::ostringstream oss;
    oss << "{"
    "\"EventId\":"          << eventId   << ","
    "\"Timestamp\":"        << timestamp << ","
    "\"EventType\":\""      << eventType << "\","
    "\"OrderId\":"          << orderId   << ","
    "\"ModifiedQuantity\":" << (modifiedQuantity ? std::to_string(*modifiedQuantity) : "null") << ","
    "\"ModifiedPrice\":"    << (modifiedPrice ? std::to_string(*modifiedPrice) : "null");
    oss << "}";
    return oss.str();
}

std::shared_ptr<OrderEventBase> PerEventScheduler::nextEvent(uint64_t /* currentTimestamp */) {
    return myGenerator();
}

std::shared_ptr<OrderEventBase> PoissonEventScheduler::nextEvent(uint64_t currentTimestamp) {
    double u = myUniform(myRng);
    if (u < myLambda)
        return myGenerator(currentTimestamp);
    return nullptr;
}
}

#endif
