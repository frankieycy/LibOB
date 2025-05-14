#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>

namespace Utils {
namespace Logger {
class LoggerBase {
public:
    LoggerBase() = default;
    LoggerBase(const std::string& logFileName, const bool logToFile = false, const bool logToConsole = false, const bool showLogTimestamp = true);
    virtual ~LoggerBase();
    virtual void log(const std::string& message);
private:
    std::string myLogFileName;
    std::ofstream myLogFile;
    bool myLogToFile = false;
    bool myLogToConsole = false;
    bool myShowLogTimestamp = true;
};
}
}

#endif
