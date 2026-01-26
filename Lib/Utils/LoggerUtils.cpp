#ifndef LOGGER_UTILS_CPP
#define LOGGER_UTILS_CPP
#include "Utils/LoggerUtils.hpp"
#include "Utils/Logger.hpp"

namespace Utils {
namespace Logger {
void LoggerStream::flush() {
    if (!myFlushed) {
        myLogger.log(myStream.str(), myLevel, myOverwriteLastLog);
        myFlushed = true;
    }
}
}
}

#endif
