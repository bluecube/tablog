#pragma once

#include "predictors.hpp"

#include <cstdint>
#include <climits>

namespace tablog::detail {

/// Class that compresses a stream of values.
template <
    typename ValueT_,
    typename Predictor = predictors::SimpleLinear<ValueT_, 2>,
>
class ValueCompressor {
public:
    using ValueT = ValueT_;

    /// Store a value into the value stream.
    template <typename Encoder>
    void write(const ValueT& value, Encoder& encoder) {
        const auto prediction = predictor.predict();

        /// the difference will always fit into an unsigned type
        /// It will be performed in a signed type first, though, which might overflow
        /// but this shouldn't matter.
        using unsigned_t = std::make_unsigned_t<ValueT>;
        predictor.feed(value);

        if (prediction == value) {
            encoder.predictor_hit_streak(1);
        } else {
            const auto [absError, predictionHigh] = abs_diff(prediction, value);
            encoder.predictor_miss(predictionHigh, absError);
        }
    }

private:
    Predictor predictor;
};

}
