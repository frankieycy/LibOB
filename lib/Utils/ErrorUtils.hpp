#ifndef ERROR_UTILS_HPP
#define ERROR_UTILS_HPP
#include <string>
#include <stdexcept>

namespace Utils {
namespace Error {
class LibException : public std::exception {
public:
    LibException(const char* message) : myMessage(message) {}
    LibException(const std::string& message) : myMessage(message) {}
    const char* what() const noexcept override { return myMessage.c_str(); }
private:
    std::string myMessage;
};

inline void LIB_THROW(const std::string& message) { throw LibException(message); }
inline void LIB_ASSERT(const bool condition, const std::string& message) { if (!condition) LIB_THROW(message); }
}
}

#endif
