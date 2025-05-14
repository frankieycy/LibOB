#ifndef UTILS_HPP
#define UTILS_HPP
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <cmath>
#include <limits>
#include "Logger.hpp"

namespace Utils {
namespace Counter {
class IdHandlerBase {
public:
    IdHandlerBase() = default;
    IdHandlerBase(const bool idLogEnabled) : myIdLogEnabled(idLogEnabled) {}
    const uint64_t getCurrentId() const { return myCurrentId; }
    const std::vector<uint64_t>& getIdLog() const { return myIdLog; }
    const uint64_t generateId();
    void reset();
private:
    uint64_t myCurrentId = 0;
    bool myIdLogEnabled = false;
    std::vector<uint64_t> myIdLog;
};

class TimestampHandlerBase {
public:
    TimestampHandlerBase() = default;
    const uint64_t getCurrentTimestamp() const { return myCurrentTimestamp; }
    const uint64_t tick(const uint64_t elapsedTimeUnit = 1);
    void reset();
private:
    uint64_t myCurrentTimestamp = 0;
};
}

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
constexpr int NAN_INT = std::numeric_limits<int>::quiet_NaN();
constexpr double NAN_DOUBLE = std::numeric_limits<double>::quiet_NaN();
constexpr double POS_INF_DOUBLE = std::numeric_limits<double>::infinity();
constexpr double NEG_INF_DOUBLE = -std::numeric_limits<double>::infinity();
inline const bool isNaN(double x) { return std::isnan(x); }
}

namespace Maths {}

namespace Statistics {}

namespace FileIO {}
}

#endif
