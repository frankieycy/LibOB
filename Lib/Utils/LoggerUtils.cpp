#ifndef LOGGER_UTILS_CPP
#define LOGGER_UTILS_CPP
#include "Utils/LoggerUtils.hpp"
#include "Utils/Logger.hpp"

namespace Utils {
namespace Logger {
std::string toString(const LogLevel& level) {
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::TRACE:   return "TRACE";
        default:                return "NULL";
    }
}

std::ostream& operator<<(std::ostream& out, const LogLevel& level) { return out << toString(level); }

void LoggerStream::flush() {
    if (!myFlushed) {
        myLogger.log(myStream.str(), myLevel, myOverwriteLastLog);
        myFlushed = true;
    }
}
}
}

#endif
