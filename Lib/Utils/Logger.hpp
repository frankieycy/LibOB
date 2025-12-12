#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>
#include "Utils/LoggerUtils.hpp"

namespace Utils {
namespace Logger {
class LoggerBase {
public:
    LoggerBase() = default;
    LoggerBase(const LoggerBase& logger) = delete; // disallow copy
    LoggerBase(const std::string& logFileName, const bool logToConsole = false, const bool showLogTimestamp = true);
    virtual ~LoggerBase();
    virtual void log(const std::string& message, const LogLevel level = LogLevel::INFO, const OverwriteLastLog overwrite = OverwriteLastLog::NO);
    virtual std::string getTimestamp() const;
    void setSilent(const bool silent) { myIsSilent = silent; }
    void setLogFile(const std::string& logFileName, const bool logToConsole = false, const bool showLogTimestamp = true) {
        myLogToFile = true;
        myLogToConsole = logToConsole;
        myShowLogTimestamp = showLogTimestamp;
        if (myLogFile.is_open())
            myLogFile.close();
        myLogFileName = logFileName;
        myLogFile.open(myLogFileName, std::ios::out);
    }
    LoggerStream operator<<(const LogLevel& level) {
        return LoggerStream(*this, level);
    }
    template<typename T>
    LoggerStream operator<<(const T& value) {
        LoggerStream stream(*this);
        stream << value;
        return stream;
    }
private:
    std::ofstream myLogFile;
    std::string myLogFileName;
    std::string myLastLogCache;
    size_t myLastLogLineCount = 0;
    bool myLogToFile = false;
    bool myLogToConsole = true;
    bool myShowLogTimestamp = true;
    bool myIsSilent = false;
};
}
}

#endif
