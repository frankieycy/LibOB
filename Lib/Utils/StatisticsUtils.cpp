#ifndef STATISTICS_UTILS_CPP
#define STATISTICS_UTILS_CPP
#include <sstream>
#include <vector>
#include <algorithm>
#include "Utils/ConstsUtils.hpp"
#include "Utils/ErrorUtils.hpp"
#include "Utils/StatisticsUtils.hpp"

namespace Utils {
namespace Statistics {
std::string toString(const VectorStats& stats) {
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
    return out << toString(stats);
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

Histogram::Histogram(double min, double max, size_t numBins, Binning binning) :
    myBins(numBins + 2, 0),
    myBinLowerEdges(numBins + 2),
    myBinUpperEdges(numBins + 2),
    myTotalCount(0),
    myBinning(binning) {
    if (numBins == 0)
        Error::LIB_THROW("[Histogram] Number of bins must be greater than zero.");
    if (min >= max)
        Error::LIB_THROW("[Histogram] Min must be less than max.");
    if (binning == Binning::UNIFORM) {
        const double binWidth = (max - min) / static_cast<double>(numBins);
        for (size_t i = 0; i < numBins; ++i) {
            myBinLowerEdges[i + 1] = min + i * binWidth;
            myBinUpperEdges[i + 1] = min + (i + 1) * binWidth;
        }
        myBinLowerEdges[0] = Consts::NEG_INF_DOUBLE;
        myBinUpperEdges[0] = myBinLowerEdges[1];
        myBinLowerEdges[numBins + 1] = myBinUpperEdges[numBins];
        myBinUpperEdges[numBins + 1] = Consts::POS_INF_DOUBLE;
    } else if (binning == Binning::LOG) {
        if (min <= 0)
            Error::LIB_THROW("[Histogram] Min must be positive for log binning.");
        const double logMin = std::log10(min);
        const double logMax = std::log10(max);
        const double logBinWidth = (logMax - logMin) / static_cast<double>(numBins);
        for (size_t i = 0; i < numBins; ++i) {
            myBinLowerEdges[i + 1] = std::pow(10.0, logMin + i * logBinWidth);
            myBinUpperEdges[i + 1] = std::pow(10.0, logMin + (i + 1) * logBinWidth);
        }
        myBinLowerEdges[0] = 0.0;
        myBinUpperEdges[0] = myBinLowerEdges[1];
        myBinLowerEdges[numBins + 1] = myBinUpperEdges[numBins];
        myBinUpperEdges[numBins + 1] = Consts::POS_INF_DOUBLE;
    } else {
        Error::LIB_THROW("[Histogram] Unsupported binning type.");
    }
}

Histogram::Histogram(const std::vector<double>& data, size_t numBins, Binning binning) :
    Histogram(*std::min_element(data.begin(), data.end()),
              *std::max_element(data.begin(), data.end()),
              numBins, binning) {
    for (const double value : data)
        add(value);
}

Histogram::Histogram(const std::vector<double>& binEdges) :
    myBins(binEdges.size() + 1, 0),
    myBinLowerEdges(binEdges.begin(), binEdges.end()),
    myBinUpperEdges(binEdges.begin(), binEdges.end()),
    myTotalCount(0),
    myBinning(Binning::CUSTOM) {
    if (binEdges.size() < 2)
        Error::LIB_THROW("[Histogram] At least two bin edges are required.");
    for (size_t i = 1; i < binEdges.size(); ++i) {
        if (binEdges[i] <= binEdges[i - 1])
            Error::LIB_THROW("[Histogram] Bin edges must be in strictly increasing order.");
    }
    myBinLowerEdges.insert(myBinLowerEdges.begin(), Consts::NEG_INF_DOUBLE);
    myBinUpperEdges.push_back(Consts::POS_INF_DOUBLE);
}

void Histogram::add(double value) {
    const size_t binIndex = getBinIndex(value);
    if (binIndex < myBins.size()) {
        ++myBins[binIndex];
        ++myTotalCount;
        mySumValues += value;
        mySumValuesSquared += value * value;
    }
}

void Histogram::clear() {
    std::fill(myBins.begin(), myBins.end(), 0);
    myTotalCount = 0;
}

size_t Histogram::getCount(size_t bin) const {
    return bin < myBins.size() ? myBins[bin] : 0;
}

size_t Histogram::getBinIndex(double value) const {
    if (myBins.size() == 0)
        Error::LIB_THROW("[Histogram::getBinIndex] Histogram has no bins.");
    else if (myBins.size() == 1)
        return 0; // single bin histogram
    if (myBinning == Binning::CUSTOM) {
        // binary search for custom bin edges
        auto it = std::lower_bound(myBinLowerEdges.begin(), myBinLowerEdges.end(), value);
        return std::distance(myBinLowerEdges.begin(), it);
    } else {
        // uniform or log binning
        if (value < myBinLowerEdges[1])
            return 0; // underflow bin
        else if (value >= myBinUpperEdges[myBins.size() - 2])
            return myBins.size() - 1; // overflow bin
        else {
            if (myBinning == Binning::UNIFORM) {
                const double binWidth = myBinUpperEdges[1] - myBinLowerEdges[1];
                size_t binIndex = static_cast<size_t>((value - myBinLowerEdges[1]) / binWidth) + 1;
                if (binIndex >= myBins.size() - 1)
                    binIndex = myBins.size() - 2; // clamp to last regular bin
                return binIndex;
            } else if (myBinning == Binning::LOG) {
                const double logMin = std::log10(myBinLowerEdges[1]);
                const double logMax = std::log10(myBinUpperEdges[myBins.size() - 2]);
                const double logBinWidth = (logMax - logMin) / static_cast<double>(myBins.size() - 2);
                size_t binIndex = static_cast<size_t>((std::log10(value) - logMin) / logBinWidth) + 1;
                if (binIndex >= myBins.size() - 1)
                    binIndex = myBins.size() - 2; // clamp to last regular bin
                return binIndex;
            } else {
                Error::LIB_THROW("[Histogram::getBinIndex] Unsupported binning type.");
            }
        }
    }
    return -1; // never reached
}

double Histogram::getBinCenter(size_t bin) const {
    return 0.5 * (getBinLower(bin) + getBinUpper(bin));
}

double Histogram::getBinLower(size_t bin) const {
    return bin < myBinLowerEdges.size() ? myBinLowerEdges[bin] : Consts::NAN_DOUBLE;
}

double Histogram::getBinUpper(size_t bin) const {
    return bin < myBinUpperEdges.size() ? myBinUpperEdges[bin] : Consts::NAN_DOUBLE;
}

double Histogram::getMean() const {
    return myTotalCount == 0 ? 0.0 : mySumValues / static_cast<double>(myTotalCount);
}

double Histogram::getVariance() const {
    if (myTotalCount == 0)
        return 0.0;
    const double mean = getMean();
    return (mySumValuesSquared / static_cast<double>(myTotalCount)) - (mean * mean);
}

std::string Histogram::getAsCsv() const {
    std::ostringstream oss;
    oss << "BinLower,BinUpper,Count,CumCount\n";
    for (size_t i = 0; i < myBins.size(); ++i)
        oss << getBinLower(i) << "," << getBinUpper(i) << "," << myBins[i] << "\n";
    return oss.str();
}

std::string Histogram::getAsJson() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < myBins.size(); ++i) {
        oss << "{"
            << "\"BinLower\":" << getBinLower(i) << ","
            << "\"BinUpper\":" << getBinUpper(i) << ","
            << "\"Count\":" << myBins[i]
            << "}";
        if (i < myBins.size() - 1)
            oss << ",";
    }
    oss << "]";
    return oss.str();
}

template <typename T>
Autocorrelation<T>::Autocorrelation(const std::vector<T>& values) :
    myValues(values) {
    mySumValues = 0.0;
    mySumValuesSquared = 0.0;
    for (const T& value : myValues) {
        const double dValue = static_cast<double>(value);
        mySumValues += dValue;
        mySumValuesSquared += dValue * dValue;
    }
}

template <typename T>
double Autocorrelation<T>::get(size_t lag) const {
    const size_t n = myValues.size();
    if (n == 0)
        Error::LIB_THROW("[Autocorrelation::get] No values added.");
    if (lag >= n)
        Error::LIB_THROW("[Autocorrelation::get] Lag is greater than or equal to number of values.");
    const double mean = getMean();
    const double variance = getVariance();
    if (variance == 0.0)
        return 0.0;
    double autocovariance = 0.0;
    for (size_t i = 0; i < n - lag; ++i)
        autocovariance += (static_cast<double>(myValues[i]) - mean) * (static_cast<double>(myValues[i + lag]) - mean);
    autocovariance /= static_cast<double>(n - lag);
    return autocovariance / variance;
}

template <typename T>
double Autocorrelation<T>::getMean() const {
    return myValues.empty() ? 0.0 : mySumValues / static_cast<double>(myValues.size());
}

template <typename T>
double Autocorrelation<T>::getVariance() const {
    if (myValues.empty())
        return 0.0;
    const double mean = getMean();
    return (mySumValuesSquared / static_cast<double>(myValues.size())) - (mean * mean);
}
}
}

#endif
