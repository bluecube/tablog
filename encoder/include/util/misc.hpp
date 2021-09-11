#pragma once

#include <type_traits>
#include <utility>

namespace tablog::detail {

template <typename T>
constexpr auto difference_helper(T a, T b) {
    if constexpr (std::is_integral_v<T>)
        return static_cast<std::make_unsigned_t<T>>(a - b);
    else
        return a - b;
}

/// Return pair of abs(a - b), a > b.
/// Does not overflow for either signed or unsigned integer types, returns unsigned value.
/// For non-integral types works as expected, without any type casts.
template <typename T>
constexpr std::pair<decltype(difference_helper(T(), T())), bool> abs_diff(T a, T b) {
    bool aHigher = (a > b);
    if (!aHigher)
        std::swap(a, b);

    return std::make_pair(difference_helper(a, b), aHigher);
}

}
