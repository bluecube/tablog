#pragma once

#include <type_traits>
#include <utility>

namespace tablog::detail {

template <typename T, bool isSigned>
struct AbsDiffTypeHelper {
    using Type = T;
};

template <typename T>
struct AbsDiffTypeHelper<T, true> {
    using Type = std::make_unsigned_t<T>;
};

template <typename T>
using AbsDiffType = typename AbsDiffTypeHelper<T, std::is_integral_v<T>>::Type;

/// Return pair of abs(a - b), a > b.
/// Does not overflow for either signed or unsigned integer types, returns unsigned value.
/// For non-integral types works as expected, without any type casts.
template <typename T>
constexpr std::pair<AbsDiffType<T>, bool> abs_diff(T a, T b) {
    bool aHigher = (a > b);
    if (!aHigher)
        std::swap(a, b);

    using T2 = AbsDiffType<T>;
    return std::make_pair(static_cast<T2>(a) - static_cast<T2>(b), aHigher);
}
}

}
