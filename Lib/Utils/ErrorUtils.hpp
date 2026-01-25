#ifndef ERROR_UTILS_HPP
#define ERROR_UTILS_HPP
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cxxabi.h>
#include <execinfo.h>

namespace Utils {
namespace Error {
inline std::string demangle(const char* mangled) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : mangled;
    std::free(demangled);
    return result;
}

inline std::string captureStackTrace(size_t maxFrames = 64) {
    void* buffer[64];
    int n = ::backtrace(buffer, static_cast<int>(maxFrames));
    char** symbols = ::backtrace_symbols(buffer, n);
    std::ostringstream oss;
    for (int i = 0; i < n; ++i) {
        std::string line(symbols[i]);
        std::istringstream iss(line);
        std::ostringstream rebuilt;
        std::string token;
        bool demangledOnce = false;
        while (iss >> token) {
            if (!demangledOnce && token.rfind("_Z", 0) == 0) {
                // token starts with "_Z" => mangled C++ symbol
                rebuilt << demangle(token.c_str()) << " ";
                demangledOnce = true;
            } else {
                rebuilt << token << " ";
            }
        }
        oss << rebuilt.str() << '\n';
    }
    std::free(symbols);
    return oss.str();
}

class LibException : public std::exception {
public:
    LibException(const char* message) : myMessage(message), myStackTrace(captureStackTrace()) {}
    LibException(const std::string& message) : myMessage(message), myStackTrace(captureStackTrace()) {}
    const char* what() const noexcept override { return myMessage.c_str(); }
    const std::string& stackTrace() const noexcept { return myStackTrace; }
private:
    std::string myMessage;
    std::string myStackTrace;
};

inline void LIB_THROW(const std::string& message) { throw LibException(message); }
inline void LIB_ASSERT(const bool condition, const std::string& message) { if (!condition) LIB_THROW(message); }
}
}

#endif
