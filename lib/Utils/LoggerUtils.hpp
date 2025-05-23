#ifndef LOGGER_UTILS_HPP
#define LOGGER_UTILS_HPP
#include <sstream>

namespace Utils {
namespace Logger {
class LoggerBase;
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE, NULL_LOG_LEVEL };

class LoggerStream {
public:
    LoggerStream(LoggerBase& logger, const LogLevel& level = LogLevel::INFO) : myLogger(logger), myLevel(level) {}
    ~LoggerStream();
    LoggerStream(LoggerStream&&) = default;
    LoggerStream& operator=(LoggerStream&&) = delete;
    LoggerStream(const LoggerStream&) = delete;
    LoggerStream& operator=(const LoggerStream&) = delete;
    template<typename T>
    LoggerStream& operator<<(const T& value) {
        myStream << value;
        return *this;
    }
private:
    LoggerBase& myLogger;
    LogLevel myLevel;
    std::ostringstream myStream;
};

std::string to_string(const LogLevel& level);
std::ostream& operator<<(std::ostream& out, const LogLevel& level);
}
}

#endif
