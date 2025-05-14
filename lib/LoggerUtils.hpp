#ifndef LOGGER_UTILS_HPP
#define LOGGER_UTILS_HPP
#include <sstream>

namespace Utils {
namespace Logger {
class LoggerBase;

class LoggerStream {
public:
    LoggerStream(LoggerBase& logger) : myLogger(logger) {}
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
    std::ostringstream myStream;
};
}
}

#endif
