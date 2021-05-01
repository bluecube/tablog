#pragma once

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
        T prediction = history[idx(4)];

        Valu += (4 * history[idx(4)] - 3 * history[idx(0)]) / 5;
        prediction -= (4 * history[idx(4)] - 3 * history[idx(0)]) / 5;

        return prediction;
    }

    /// Feed a new value to the predictor
    void feed(T value) {
        for (uint_fast8_t i = 0; i < values.size(); ++i) {
            auto prevDerivative = i < (values.size() - 1) ? smoothedDerivatives[i + 1] : 0;

            auto nextDerivative = 
            smoothedDerivatives[i] += 

            smoothedDerivatives[i] =
                smoothedDerivatives[i] + prevDerivative \
                - (value >> smoothingPower) \
                - (smoothedDerivatives[i]) >> smoothingPower \- nextDerivative >> smoothingPower;
        }
    }

protected:
    /// Index into history array, parameter 0 corresponds to the oldest value,
    /// 4 to the newest.
    uint_fast8_t idx(uint_fast8_t i) {
        return (oldestHistoryIndex + i) % history.size();
    }

    util::CircularBuffer<T, 5, uint_fast8_t> history;
};



