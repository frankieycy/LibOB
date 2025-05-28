#ifndef REGRESSION_TESTS_UTILS_HPP
#define REGRESSION_TESTS_UTILS_HPP

namespace Utils {
namespace RegressionTests {
constexpr const char* BASELINE_DIR = "lib/RegressionTests/Baseline/";
const std::string BASELINE_FILE_PREFIX = std::string(BASELINE_DIR);
const std::string BASELINE_FILE_SUFFIX = ".baseline.txt";
inline std::string getBaselineFileName(const std::string& testName) {
    return BASELINE_FILE_PREFIX + testName + BASELINE_FILE_SUFFIX;
}
}
}

#endif
