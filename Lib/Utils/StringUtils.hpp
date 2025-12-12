#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace Utils {
namespace String {
template <std::size_t N>
void stringToCharRaw(const std::string& str, char (&out)[N], char padding = ' ') {
    static_assert(N > 0, "Output array size must be greater than zero.");
    std::memset(out, padding, N);
    std::memcpy(out, str.data(), std::min(str.size(), N));
}

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
