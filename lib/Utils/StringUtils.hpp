#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP
#include <string>
#include <cstdint>
#include <type_traits>

namespace Utils {
namespace String {
template<typename T>
T packStringTo(const std::string& str) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    T result = 0;
    size_t len = std::min(str.size(), sizeof(T));
    for (size_t i = 0; i < len; ++i) {
        result |= static_cast<T>(str[i]) << (8 * (len - 1 - i));
    }
    return result;
}

template<typename T>
T hashStringTo(const std::string& str) {
    // FNV-1a 64-bit hash function (adjustable for other types)
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    uint64_t hash = 14695981039346656037ull; // FNV offset basis
    for (char c : str) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ull; // FNV prime
    }
    return static_cast<T>(hash);
}
}
}

#endif
