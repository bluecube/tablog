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

}
