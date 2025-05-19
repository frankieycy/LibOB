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
#include <random>
#include <limits>
#include <optional>
#include "Utils/Logger.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& out, std::vector<T>& vec) {
    out << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        out << vec[i];
        if (i != vec.size() - 1)
            out << ", ";
    }
    out << "]";
    return out;
}

namespace Utils {
namespace Counter {
class IdHandlerBase {
public:
    IdHandlerBase() = default;
    IdHandlerBase(const bool idLogEnabled) : myIdLogEnabled(idLogEnabled) {}
    const std::vector<uint64_t>& getIdLog() const { return myIdLog; }
    uint64_t getCurrentId() const { return myCurrentId; }
    uint64_t generateId();
    void reset();
private:
    uint64_t myCurrentId = 0;
    bool myIdLogEnabled = false;
    std::vector<uint64_t> myIdLog;
};

class TimestampHandlerBase {
public:
    TimestampHandlerBase() = default;
    uint64_t getCurrentTimestamp() const { return myCurrentTimestamp; }
    uint64_t tick(const uint64_t elapsedTimeUnit = 1);
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
inline bool isNaN(double x) { return std::isnan(x); }
}

namespace Vector {
std::vector<double> getVectorRange(const double a, const double b, const double x);
std::vector<double> getVectorRange(const double a, const double b, const int n);
}

namespace Maths {
inline double roundPriceToTick(double price, double tick = 0.01) {
    const auto ticks = std::llround(price / tick);
    return ticks * tick;
}
}

namespace Statistics {
inline std::mt19937& GLOBAL_RNG() {
    static thread_local std::mt19937 eng{ std::random_device{}() };
    return eng;
}

inline std::mt19937& RNG_42() {
    static thread_local std::mt19937 eng{ 42 };
    return eng;
}

template<class Engine>
inline double getRandomUniform01(Engine& eng) {
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(eng);
}

inline double getRandomUniform01(const bool deterministic = false) { return deterministic ? getRandomUniform01(RNG_42()) : getRandomUniform01(GLOBAL_RNG()); }

inline double getRandomUniform(const double a, const double b, const bool deterministic = false) { return a + (b - a) * getRandomUniform01(deterministic); }

template<class Engine, class Int>
inline int getRandomUniformInt(const Int a, const Int b, Engine& eng) {
    static thread_local std::uniform_int_distribution<Int> dist(a, b);
    return dist(eng);
}

template<class Int>
inline int getRandomUniformInt(const Int a, const Int b, const bool deterministic = false) { return deterministic ? getRandomUniformInt(a, b, RNG_42()) : getRandomUniformInt(a, b, GLOBAL_RNG()); }
}

namespace FileIO {}
}

#endif
