#ifndef STATISTICS_UTILS_HPP
#define STATISTICS_UTILS_HPP
#include <vector>
#include <deque>
#include <random>

namespace Utils {
namespace Statistics {
struct VectorStats {
    size_t size;
    double mean;
    double variance;
    double stddev;
};

template<typename T>
class TimeSeriesCollector {
public:
    void addSample(std::shared_ptr<const T> stats) {
        mySamples.push_back(stats);
        if (myMaxHistory > 0 && mySamples.size() > myMaxHistory)
            mySamples.pop_front();
    }
    const std::deque<std::shared_ptr<const T>>& getSamples() const { return mySamples; }
    std::shared_ptr<const T> getLastSample() const { return mySamples.empty() ? nullptr : mySamples.back(); }
    size_t size() const { return mySamples.size(); }
    void setMaxHistory(size_t maxSamples) { myMaxHistory = maxSamples; }
    void clear() { mySamples.clear(); }
private:
    std::deque<std::shared_ptr<const T>> mySamples;
    size_t myMaxHistory = 0;
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
    std::uniform_int_distribution<Int> dist(a, b);
    return dist(eng);
}

template<class Engine, class Int>
inline Int getRandomUniformIntFast(const Int a, const Int b, Engine& eng) {
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    const double u = dist(eng);
    Int result = static_cast<Int>(a + std::floor(u * (b - a + 1)));
    return (result > b) ? b : result;
}

template<class Int>
inline int getRandomUniformInt(const Int a, const Int b, const bool deterministic = false) { return deterministic ? getRandomUniformIntFast(a, b, RNG_42()) : getRandomUniformIntFast(a, b, GLOBAL_RNG()); }

template<class T>
inline T drawRandomElement(const std::vector<T>& vec, const bool deterministic = false) {
    if (vec.empty())
        Error::LIB_THROW("[drawRandomElement] Empty vector.");
    return vec[getRandomUniformInt(0, static_cast<int>(vec.size()) - 1, deterministic)];
}

template<typename Container>
auto drawRandomIterator(const Container& container, const bool deterministic = false) {
    if (container.empty())
        Error::LIB_THROW("[drawRandomIterator] Empty container.");
    const size_t index = getRandomUniformInt(0, static_cast<int>(container.size()) - 1, deterministic);
    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<typename Container::const_iterator>::iterator_category>)
        return container.begin() + index; // specialization for random access iterators e.g. std::vector, std::deque
    else
        return std::next(container.begin(), index);
}

size_t drawIndexWithRelativeProbabilities(const std::vector<double>& probabilities, const bool deterministic = false);

template<typename T>
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
