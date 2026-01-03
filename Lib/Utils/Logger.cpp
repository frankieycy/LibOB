#ifndef LOGGER_CPP
#define LOGGER_CPP
#include <string>
#include <iostream>
#include <fstream>
#include "Utils/Logger.hpp"

namespace Utils {
namespace Logger {
LoggerBase::LoggerBase(const std::string& logFileName, const bool logToConsole, const bool showLogTimestamp) :
    myLogFileName(logFileName), myLogToFile(true), myLogToConsole(logToConsole), myShowLogTimestamp(showLogTimestamp) {
    if (myLogToFile) {
        myLogFile.open(myLogFileName, std::ios::out);
        if (!myLogFile.is_open())
            throw std::runtime_error("LoggerBase: failed to open log file " + myLogFileName);
    }
}

LoggerBase::~LoggerBase() {
    if (myLogToFile && myLogFile.is_open())
        myLogFile.close();
}

void LoggerBase::log(const std::string& message, const LogLevel level, const OverwriteLastLog overwrite) {
    if (myIsSilent)
        return;
    const std::string timestampStr = myShowLogTimestamp ? ('[' + getTimestamp() + ']') : "[LOG]";
    const std::string logMessage = timestampStr + " " + toString(level) + " " + message;
    if (myLogToConsole) {
        if (overwrite == OverwriteLastLog::YES)
            for (size_t i = 0; i < myLastLogLineCount; ++i)
                std::cout << "\x1b[1A"  // move up one line
                          << "\x1b[2K"; // clear the line
        std::cout << logMessage << std::endl;
    }
    if (myLogToFile && myLogFile.is_open())
        myLogFile << logMessage << std::endl;
    myLastLogCache = logMessage;
    myLastLogLineCount = std::count(logMessage.begin(), logMessage.end(), '\n') + 1;
}

std::string LoggerBase::getTimestamp() const {
    const std::time_t now = std::time(nullptr);
    const std::tm* localTime = std::localtime(&now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer);
}
}
}

#endif
