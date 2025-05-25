#ifndef CONSTS_UTILS_HPP
#define CONSTS_UTILS_HPP
#include <cmath>
#include <limits>

namespace Utils {
namespace Consts {
constexpr double NAN_DOUBLE = std::numeric_limits<double>::quiet_NaN();
constexpr double POS_INF_DOUBLE = std::numeric_limits<double>::infinity();
constexpr double NEG_INF_DOUBLE = -std::numeric_limits<double>::infinity();
template<typename T>
constexpr T quietNaN() {
    LIB_ASSERT(std::is_floating_point_v<T>, "quietNaN() is only defined for floating-point types");
    return std::numeric_limits<T>::quiet_NaN();
}
inline bool isNaN(double x) { return std::isnan(x); }
}
}

#endif
