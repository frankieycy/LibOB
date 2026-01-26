#ifndef ENUM_STRINGS_HPP
#define ENUM_STRINGS_HPP
#include <string>
#include <type_traits>

namespace Utils {
template<typename E>
struct EnumStrings;

template<typename E>
    requires std::is_enum_v<E>
inline std::string toString(E value) {
    using U = std::underlying_type_t<E>;
    constexpr auto& names = EnumStrings<E>::names;
    auto idx = static_cast<U>(value);
    if (idx < 0 || static_cast<size_t>(idx) >= names.size())
        return "<INVALID_ENUM>";
    return names[idx];
}

template<typename E>
    requires std::is_enum_v<E>
std::ostream& enumStream(std::ostream& os, E value) {
    return os << toString(value);
}
}

#endif
