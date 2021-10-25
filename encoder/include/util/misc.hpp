#pragma once

#include <type_traits>
#include <utility>
#include <limits>
#include <cstdint>
#include <cassert>

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

/// Extrapolate a new sample from two previous samples.
/// Calculates last + (last - first) / divisor.
/// If the theoretical infinite-precision result fits into the output, it is
/// returned without, overflows, if it doesn't the result is saturated at the
/// type min or max.
/// @param first first sample
/// @param last last sample
/// @param divisor distance between sampled values. Must be >= 2
template <typename T, T divisor=2>
constexpr T extrapolate(T first, T last) noexcept {
    static_assert(divisor >= 2);
    T ret;
    auto [difference, decreasing] = abs_diff(first, last);
    auto change = static_cast<T>(difference / divisor);
    if (decreasing) {
        bool overflow = __builtin_sub_overflow(last, change, &ret);
        if (overflow)
          return std::numeric_limits<T>::min();
    } else {
        bool overflow = __builtin_add_overflow(last, change, &ret);
        if (overflow)
          return std::numeric_limits<T>::max();
    }
    return ret;
}

/// Calculate floor(log2(v)) for values between 1 and 9 (inclusive).
constexpr uint_fast8_t small_int_log2(uint_fast8_t v) {
    return (v >> 1) - (v > 5);
}

/// Calculate floor(log2(v)) for values between 1 and 9 (inclusive).
template <typename T>
constexpr uint_fast8_t int_log2(T v) {
    static_assert(!std::is_signed_v<T>);
    assert(v > 0);

    if constexpr (std::is_same_v<T, unsigned long long>)
        return std::numeric_limits<T>::digits - 1 - __builtin_clzll(v);
    else if constexpr (std::is_same_v<T, unsigned long>)
        return std::numeric_limits<T>::digits - 1 - __builtin_clzl(v);
    else
        return std::numeric_limits<unsigned>::digits - 1 - __builtin_clz(v);
}

}
