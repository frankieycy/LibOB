#ifndef STATISTICS_UTILS_HPP
#define STATISTICS_UTILS_HPP
#include <vector>
#include <random>

namespace Utils {
namespace Statistics {
struct VectorStats {
    size_t size;
    double mean;
    double variance;
    double stddev;
};

std::string to_string(const VectorStats& stats);
std::ostream& operator<<(std::ostream& out, const VectorStats& orderMatchingStrategy);

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

template<class T>
inline T drawRandomElement(const std::vector<T>& vec, const bool deterministic = false) {
    if (vec.empty())
        Error::LIB_THROW("[drawRandomElement] Empty vector.");
    return vec[getRandomUniformInt(0, static_cast<int>(vec.size()) - 1, deterministic)];
}

template <typename Container>
auto drawRandomIterator(Container& container, const bool deterministic = false) {
    if (container.empty())
        Error::LIB_THROW("[getRandomIterator] Empty container.");
    auto it = container.begin();
    std::advance(it, getRandomUniformInt(0, static_cast<int>(container.size()) - 1, deterministic));
    return it;
}

size_t drawIndexWithRelativeProbabilities(const std::vector<double>& probabilities, const bool deterministic = false);

template <typename T>
VectorStats getVectorStats(const std::vector<T>& vec) {
    if (vec.empty())
        Error::LIB_THROW("[getVectorStats] Empty vector.");
    VectorStats stats;
    stats.size = vec.size();
    stats.mean = std::accumulate(vec.begin(), vec.end(), 0.0,
        [](double acc, const T& val) {
            return acc + static_cast<double>(val);
        }) / stats.size;
    stats.variance = std::accumulate(vec.begin(), vec.end(), 0.0,
        [mean = stats.mean](double acc, const T& val) {
            double d = static_cast<double>(val);
            return acc + (d - mean) * (d - mean);
        }) / stats.size;
    stats.stddev = std::sqrt(stats.variance);
    return stats;
}
}
}

#endif
