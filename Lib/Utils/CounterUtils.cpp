#ifndef COUNTER_UTILS_CPP
#define COUNTER_UTILS_CPP
#include <vector>
#include "Utils/CounterUtils.hpp"

namespace Utils {
namespace Counter {
uint64_t IdHandlerBase::generateId() {
    if (myIdLogEnabled)
        myIdLog.push_back(myCurrentId);
    return myCurrentId++;
}

void IdHandlerBase::reset() {
    myCurrentId = 0;
    myIdLog.clear();
}

uint64_t TimestampHandlerBase::tick(const uint64_t elapsedTimeUnit) {
    myCurrentTimestamp += elapsedTimeUnit;
    return myCurrentTimestamp;
}

void TimestampHandlerBase::reset() {
    myCurrentTimestamp = 0;
}
}
}

#endif
