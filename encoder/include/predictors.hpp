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
    T predict() const {
        return detail::extrapolate<T, N - 1>(history.front(), history.back());
    }

    /// Feed a new value to the predictor
    void feed(T value) {
        history.push_back(value);
    }

protected:
    util::CircularBufferFixed<T, N, uint_fast8_t> history;
};

}
