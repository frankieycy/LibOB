#ifndef LOGGER_UTILS_HPP
#define LOGGER_UTILS_HPP
#include <sstream>

namespace Utils {
namespace Logger {
class LoggerBase;
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE, NULL_LOG_LEVEL };
enum class OverwriteLastLog : bool { YES = true, NO = false };

class LoggerStream {
public:
    LoggerStream(LoggerBase& logger, const LogLevel level = LogLevel::INFO) : myLogger(logger), myLevel(level) {}
    ~LoggerStream() { flush(); };
    LoggerStream(LoggerStream&&) = default;
    LoggerStream& operator=(LoggerStream&&) = delete;
    LoggerStream(const LoggerStream&) = delete;
    LoggerStream& operator=(const LoggerStream&) = delete;
    LoggerStream& operator<<(OverwriteLastLog overwrite) {
        myOverwriteLastLog = overwrite;
        return *this;
    }
    template<typename T>
    LoggerStream& operator<<(const T& value) {
        myStream << value;
        return *this;
    }
    void flush();
private:
    std::ostringstream myStream;
    LoggerBase& myLogger;
    LogLevel myLevel;
    OverwriteLastLog myOverwriteLastLog = OverwriteLastLog::NO;
    bool myFlushed = false;
};

std::string toString(const LogLevel& level);
std::ostream& operator<<(std::ostream& out, const LogLevel& level);
}
}

#endif
