#pragma once

#include "util/circular_buffer.hpp"

#include <array>
#include <numeric>
#include <cstdint>
#include <cstddef>

namespace tablog::predictors {

/// Predicts next value by looking at the average derivative of the last N values
template <typename T, size_t N>
class SimpleLinear {
public:
    /// Returns a predicted value
    T predict() {
        T prediction = history.back();
        prediction += (history.back() - history.front()) / T(N - 1);
        return prediction;
    }

    /// Feed a new value to the predictor
    void feed(T value) {
        history.push_back(value);
    }

protected:
    util::CircularBuffer<T, N, uint_fast8_t> history;
};



