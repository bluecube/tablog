#pragma once

#include "util/circular_buffer.hpp"
#include "util/misc.hpp"

#include <array>
#include <numeric>
#include <cstdint>

namespace tablog::predictors {

/// Keeps a saturating counter that switches between two modes:
/// 1. Predict no change of value
/// 2. Predict a constant first derivative based on last two values
/// Predicts next value by looking at the last two values
template <typename T>
class Linear12Adapt {
public:
    using Type = T;

    /// Returns a predicted value
    /// Prediction always works, regardless of number of data points provided.
    T predict_and_feed(T value) {
        using UnsignedT = std::make_unsigned_t<T>;
        const T prediction1 = prev[1];
        const T prediction2 = static_cast<T>(
            2u * static_cast<UnsignedT>(prev[1]) - static_cast<UnsignedT>(prev[0])
        );
        // Might overflow, but that's ok. We're avoiding UB here by going through
        // unsigned values.
        // Cast from unsigned to signed should be fine according to the standard,
        // but we still might get bitten by the cast from unsigned to signed
        // (which is only implementation defined) if trying to run on a platform
        // where overflowing cast from unsigned to signed is not two's complement...


        const auto prediction = selector >= 0 ? prediction1 : prediction2;

        const auto error1 = detail::abs_diff(prediction1, value);
        const auto error2 = detail::abs_diff(prediction2, value);

        if (error1.first < error2.first) {
            if (selector < (selectorLimit - 1))
                ++selector;
        } else if (error1.first > error2.first) {
            if (selector > -selectorLimit)
                --selector;
        }

        prev[0] = prev[1];
        prev[1] = value;

        return prediction;
    }

protected:
    static constexpr int8_t selectorLimit = 8;
    T prev[2] = {0, 0};
    int8_t selector = 0;
};

}
