#ifndef MATCHING_ENGINE_UTILS_CPP
#define MATCHING_ENGINE_UTILS_CPP
#include "Utils.hpp"
#include "MatchingEngineUtils.hpp"

namespace Exchange {
std::ostream& operator<<(std::ostream& out, const OrderMatchingStrategy& orderMatchingStrategy) {
    switch (orderMatchingStrategy) {
        case OrderMatchingStrategy::FIFO:            out << "FIFO";           break;
        case OrderMatchingStrategy::PRO_RATA:        out << "ProRata";        break;
        case OrderMatchingStrategy::ICEBERG_SUPPORT: out << "IcebergSupport"; break;
        default:                                     out << "Null";           break;
    }
    return out;
}
}

#endif
