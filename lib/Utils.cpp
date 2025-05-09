#ifndef UTILS_CPP
#define UTILS_CPP
#include "Utils.hpp"

namespace Utils {
namespace Counter {
const uint64_t IdHandlerBase::generateId() {
    ++myCurrentId;
    myIdLog.push_back(myCurrentId);
    return myCurrentId;
}

void IdHandlerBase::reset() {
    myCurrentId = 0;
    myIdLog.clear();
}

void TimestampHandlerBase::tick(const uint64_t elapsedTimeUnit) {
    myCurrentTimestamp += elapsedTimeUnit;
}

void TimestampHandlerBase::reset() {
    myCurrentTimestamp = 0;
}
}
}

#endif
