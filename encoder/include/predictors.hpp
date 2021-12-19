#pragma once

#include "util/circular_buffer.hpp"
#include "util/misc.hpp"

#include <array>
#include <numeric>
#include <cstdint>

namespace tablog::predictors {

/// Predicts next value by looking at the average derivative of the last N values
template <typename T, std::size_t N>
class SimpleLinear {
public:
    using Type = T;

    /// Returns a predicted value
    /// Prediction always works, regardless of number of data points provided.
    T predict_and_feed(T value) {
        auto prediction = detail::extrapolate<T, N - 1>(history.front(), history.back());
        history.push_back(value);
        return prediction;
    }

protected:
    util::CircularBufferFixed<T, N, uint_fast8_t> history;
};

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
        const T prediction1 = prev[1];
        const T prediction2 = 2 * prev[1] - prev[0]; // Might overflow, but that's ok.

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
