#ifndef LOGGER_UTILS_HPP
#define LOGGER_UTILS_HPP
#include <sstream>

namespace Utils {
namespace Logger {
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE };
class LoggerBase;

std::ostream& operator<<(std::ostream& out, const LogLevel& level);

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
}
}

#endif
