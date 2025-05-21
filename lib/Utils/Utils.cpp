#ifndef UTILS_CPP
#define UTILS_CPP
#include "Utils/Utils.hpp"

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

namespace Vector {
std::vector<double> getVectorRange(const double a, const double b, const double x) {
    Error::LIB_ASSERT((a <= b && x > 0) || (a >= b && x < 0), "[getVectorRange] Invalid range.");
    std::vector<double> vec;
    for (double i = a; i <= b; i += x)
        vec.push_back(i);
    return vec;
}
std::vector<double> getVectorRange(const double a, const double b, const int n) {
    Error::LIB_ASSERT(n > 0, "[getVectorRange] n must be positive.");
    std::vector<double> vec;
    const double x = (b - a) / n;
    for (int i = 0; i <= n; ++i)
        vec.push_back(a + i * x);
    return vec;
}
}
}

#endif
