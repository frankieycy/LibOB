#ifndef LOGGER_UTILS_HPP
#define LOGGER_UTILS_HPP
#include <sstream>

namespace Utils {
namespace Logger {
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE, NULL_LOG_LEVEL };
class LoggerBase;

inline std::string to_string(const LogLevel& level) {
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::TRACE:   return "TRACE";
        default:                return "NULL";
    }
}

inline std::ostream& operator<<(std::ostream& out, const LogLevel& level) { return out << to_string(level); }

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
