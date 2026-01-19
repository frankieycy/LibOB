#ifndef CONSTS_UTILS_HPP
#define CONSTS_UTILS_HPP
#include <cmath>
#include <limits>
#include "Utils/ErrorUtils.hpp"

namespace Utils {
namespace Consts {
constexpr double NAN_DOUBLE = std::numeric_limits<double>::quiet_NaN();
constexpr double POS_INF_DOUBLE = std::numeric_limits<double>::infinity();
constexpr double NEG_INF_DOUBLE = -std::numeric_limits<double>::infinity();

template<typename T>
constexpr T quietNaN() {
    Error::LIB_ASSERT(std::is_floating_point_v<T>, "quietNaN() is only defined for floating-point types");
    return std::numeric_limits<T>::quiet_NaN();
}

template <typename T>
inline bool isNaN(T x) {
    Error::LIB_ASSERT(std::is_floating_point_v<T>, "isNaN() is only defined for floating-point types");
    return std::isnan(x);
}

template <typename T>
inline bool isFinite(T x) {
    Error::LIB_ASSERT(std::is_floating_point_v<T>, "isFinite() is only defined for floating-point types");
    return std::isfinite(x);
}

template <typename T>
inline bool isValid(T x) {
    return isFinite(x);
}

template <typename T>
inline bool almostEqual(
    T a,
    T b,
    T absTol = std::numeric_limits<T>::epsilon(),
    T relTol = std::numeric_limits<T>::epsilon()) {
    return std::abs(a - b) <= std::max(absTol, relTol * std::max(std::abs(a), std::abs(b)));
}

inline bool isZero(double x) { return almostEqual(x, 0.0); }
}
}

#endif
