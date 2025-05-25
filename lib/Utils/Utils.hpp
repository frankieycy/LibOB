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
#include <thread>
#include <chrono>
#include <functional>
#include "Utils/Logger.hpp"
#include "Utils/ErrorUtils.hpp"
#include "Utils/IOUtils.hpp"
#include "Utils/CounterUtils.hpp"
#include "Utils/ConstsUtils.hpp"
#include "Utils/MathsUtils.hpp"
#include "Utils/StatisticsUtils.hpp"
#include "Utils/VectorUtils.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& out, std::vector<T>& vec) {
    out << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        out << vec[i];
        if (i != vec.size() - 1)
            out << ", ";
    }
    return out << "]";
}

#endif
