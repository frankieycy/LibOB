#ifndef MATHS_UTILS_HPP
#define MATHS_UTILS_HPP
#include <cmath>

namespace Utils {
namespace Maths {
inline double roundPriceToTick(double price, double tick = 0.01) {
    const auto ticks = std::llround(price / tick);
    return ticks * tick;
}
}
}

#endif
