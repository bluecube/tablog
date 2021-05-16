#pragma once

#include "util/circular_buffer.hpp"

#include <array>
#include <numeric>
#include <cstdint>

namespace tablog::predictors {

/// Predicts next value by looking at the average derivative of the last N values
template <typename T, std::size_t N>
class SimpleLinear {
public:
    /// Returns a predicted value
    /// Prediction always works, regardless of number of data points provided.
    ///    - returns zero if no data
    ///    - returns the first data point if there is only one
    T predict() {
        if (history.empty())
            return 0;

        T prediction = history.back();

        if (history.size() > 1)
            prediction += (history.back() - history.front()) / T(history.size() - 1);

        return prediction;
    }

    /// Feed a new value to the predictor
    void feed(T value) {
        history.push_back(value);
    }

protected:
    util::CircularBuffer<T, N, uint_fast8_t> history;
};

}
