#ifndef STATISTICS_UTILS_CPP
#define STATISTICS_UTILS_CPP
#include <sstream>
#include <vector>
#include <algorithm>
#include "Utils/ErrorUtils.hpp"
#include "Utils/StatisticsUtils.hpp"

namespace Utils {
namespace Statistics {
std::string to_string(const VectorStats& stats) {
    std::ostringstream oss;
    oss << "{"
        << "\"size\":" << stats.size << ","
        << "\"mean\":" << stats.mean << ","
        << "\"variance\":" << stats.variance << ","
        << "\"stddev\":" << stats.stddev
        << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& out, const VectorStats& stats) {
    return out << to_string(stats);
}

size_t drawIndexWithRelativeProbabilities(const std::vector<double>& probabilities, const bool deterministic) {
    if (probabilities.empty())
        Error::LIB_THROW("[drawIndexWithRelativeProbability] Empty probabilities vector.");
    if (std::any_of(probabilities.begin(), probabilities.end(), [](double p) { return p < 0.0; }))
        Error::LIB_THROW("[drawIndexWithRelativeProbability] Probabilities must be non-negative.");
    const double sum = std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
    if (sum == 0.0)
        Error::LIB_THROW("[drawIndexWithRelativeProbability] Sum of probabilities is zero.");
    std::vector<double> normalizedProbabilities(probabilities.size());
    std::transform(probabilities.begin(), probabilities.end(), normalizedProbabilities.begin(), [sum](double p) { return p / sum; });
    const double uniformRandom = Statistics::getRandomUniform01(deterministic);
    double cumulativeProbability = 0.0;
    for (size_t i = 0; i < normalizedProbabilities.size(); ++i) {
        cumulativeProbability += normalizedProbabilities[i];
        if (uniformRandom <= cumulativeProbability)
            return i;
    }
    return -1; // never reached
}
}
}

#endif
