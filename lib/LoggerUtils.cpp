#ifndef LOGGER_UTILS_CPP
#define LOGGER_UTILS_CPP
#include "LoggerUtils.hpp"
#include "Logger.hpp"

namespace Utils {
namespace Logger {
LoggerStream::~LoggerStream() {
    myLogger.log(myStream.str(), myLevel);
}
}
}

#endif
