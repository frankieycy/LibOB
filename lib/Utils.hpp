#ifndef UTILS_HPP
#define UTILS_HPP
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Utils {
namespace Error {
class LibException : public std::exception {
public:
    LibException(const char* message) : myMessage(message) {}
    LibException(const std::string& message) : myMessage(message) {}
    const char* what() const noexcept override { return myMessage.c_str(); }
private:
    std::string myMessage;
};

inline void LIB_THROW(const char* message) { throw LibException(message); }
}

namespace Consts {
constexpr double NAN_INT = std::numeric_limits<int>::quiet_NaN();
constexpr double NAN_DOUBLE = std::numeric_limits<double>::quiet_NaN();
constexpr double POS_INF_DOUBLE = std::numeric_limits<double>::infinity();
constexpr double NEG_INF_DOUBLE = -std::numeric_limits<double>::infinity();
inline const bool isNaN(double x) { return std::isnan(x); }
}

namespace Statistics {}

namespace FileIO {}
}

#endif
