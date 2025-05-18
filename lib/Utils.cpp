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

namespace Vector {
std::vector<double> getVectorRange(const double a, const double b, const double x) {
    // TODO: assert that (a <= b && x > 0) or (a >= b && x < 0)
    std::vector<double> vec;
    for (double i = a; i <= b; i += x)
        vec.push_back(i);
    return vec;
}
std::vector<double> getVectorRange(const double a, const double b, const int n) {
    // TODO: assert that n > 0
    std::vector<double> vec;
    const double x = (b - a) / n;
    for (int i = 0; i <= n; ++i)
        vec.push_back(a + i * x);
    return vec;
}
}
}

#endif
