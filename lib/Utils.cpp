#ifndef UTILS_CPP
#define UTILS_CPP
#include "Utils.hpp"

namespace Utils {
namespace Counter {
const uint64_t IdHandlerBase::generateId() {
    if (myIdLogEnabled)
        myIdLog.push_back(myCurrentId);
    return myCurrentId++;
}

void IdHandlerBase::reset() {
    myCurrentId = 0;
    myIdLog.clear();
}

const uint64_t TimestampHandlerBase::tick(const uint64_t elapsedTimeUnit) {
    myCurrentTimestamp += elapsedTimeUnit;
    return myCurrentTimestamp;
}

void TimestampHandlerBase::reset() {
    myCurrentTimestamp = 0;
}
}
}

#endif
