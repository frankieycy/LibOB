#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>
#include "LoggerUtils.hpp"

namespace Utils {
namespace Logger {
enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE };

class LoggerBase {
public:
    LoggerBase() = default;
    LoggerBase(const std::string& logFileName, const bool logToFile = true, const bool logToConsole = false, const bool showLogTimestamp = true);
    virtual ~LoggerBase();
    virtual void log(const std::string& message);
    virtual std::string getTimestamp() const;
    template<typename T>
    LoggerStream operator<<(const T& value) {
        LoggerStream stream(*this);
        stream << value;
        return stream;
    }
private:
    std::string myLogFileName;
    std::ofstream myLogFile;
    bool myLogToFile = false;
    bool myLogToConsole = true;
    bool myShowLogTimestamp = true;
};
}
}

#endif
