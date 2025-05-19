#ifndef LOGGER_CPP
#define LOGGER_CPP
#include <string>
#include <iostream>
#include <fstream>
#include "Utils/Logger.hpp"

namespace Utils {
namespace Logger {
LoggerBase::LoggerBase(const std::string& logFileName, const bool logToFile, const bool logToConsole, const bool showLogTimestamp) :
    myLogFileName(logFileName), myLogToFile(logToFile), myLogToConsole(logToConsole), myShowLogTimestamp(showLogTimestamp) {
    if (myLogToFile) {
        myLogFile.open(myLogFileName, std::ios::out | std::ios::app);
        if (!myLogFile.is_open())
            throw std::runtime_error("LoggerBase: failed to open log file " + myLogFileName);
    }
}

LoggerBase::~LoggerBase() {
    if (myLogToFile && myLogFile.is_open())
        myLogFile.close();
}

void LoggerBase::log(const std::string& message, const LogLevel& level) {
    std::string timestampStr = myShowLogTimestamp ? ('[' + getTimestamp() + ']') : "";
    if (myLogToConsole)
        std::cout << timestampStr << " " << level << " " << message << std::endl;
    if (myLogToFile && myLogFile.is_open())
        myLogFile << timestampStr << " " << level << " " << message << std::endl;
}

std::string LoggerBase::getTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer);
}
}
}

#endif
