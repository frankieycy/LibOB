#ifndef MATHS_UTILS_HPP
#define MATHS_UTILS_HPP
#include <cmath>

namespace Utils {
namespace Maths {
inline double roundPriceToTick(double price, double tick = 0.01) {
    const auto ticks = std::llround(price / tick);
    return ticks * tick;
}

template <typename T>
inline T castDoublePriceAsInt(double price, double multiplier = 10000.0) {
    return static_cast<T>(std::llround(price * multiplier));
}
}
}

#endif
