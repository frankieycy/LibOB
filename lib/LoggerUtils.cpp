#ifndef LOGGER_UTILS_CPP
#define LOGGER_UTILS_CPP
#include "LoggerUtils.hpp"
#include "Logger.hpp"

namespace Utils {
namespace Logger {
std::ostream& operator<<(std::ostream& out, const LogLevel& level) {
    switch (level) {
        case LogLevel::INFO:    out << "INFO";    break;
        case LogLevel::WARNING: out << "WARNING"; break;
        case LogLevel::ERROR:   out << "ERROR";   break;
        case LogLevel::DEBUG:   out << "DEBUG";   break;
        case LogLevel::TRACE:   out << "TRACE";   break;
        default:                out << "NULL";    break;
    }
    return out;
}

LoggerStream::~LoggerStream() {
    myLogger.log(myStream.str(), myLevel);
}
}
}

#endif
