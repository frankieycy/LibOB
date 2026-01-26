#ifndef VECTOR_UTILS_CPP
#define VECTOR_UTILS_CPP
#include <vector>
#include "Utils/ErrorUtils.hpp"

namespace Utils {
namespace Vector {
std::vector<double> getVectorRange(const double a, const double b, const double x) {
    Utils::Error::LIB_ASSERT((a <= b && x > 0) || (a >= b && x < 0), "[getVectorRange] Invalid range.");
    std::vector<double> vec;
    for (double i = a; i <= b; i += x)
        vec.push_back(i);
    return vec;
}

std::vector<double> getVectorRange(const double a, const double b, const int n) {
    Utils::Error::LIB_ASSERT(n > 0, "[getVectorRange] n must be positive.");
    std::vector<double> vec;
    const double x = (b - a) / n;
    for (int i = 0; i <= n; ++i)
        vec.push_back(a + i * x);
    return vec;
}
}
}

#endif
