#ifndef UTILS_HPP
#define UTILS_HPP
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
#include <limits>
#include <thread>
#include <chrono>
#include <optional>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "Utils/Logger.hpp"
#include "Utils/ErrorUtils.hpp"
#include "Utils/IOUtils.hpp"
#include "Utils/CounterUtils.hpp"
#include "Utils/ConstsUtils.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/MathsUtils.hpp"
#include "Utils/StatisticsUtils.hpp"
#include "Utils/VectorUtils.hpp"
#include "Utils/RegressionTestsUtils.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
    out << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        out << vec[i];
        if (i != vec.size() - 1)
            out << ", ";
    }
    return out << "]";
}

#endif
