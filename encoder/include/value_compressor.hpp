#pragma once

#include "predictors.hpp"
#include "util/misc.hpp"
#include "stream_encoder_bits.hpp"

#include <cstdint>

namespace tablog::detail {

/// Class that compresses a stream of values.
template <
    typename ValueT_,
    typename Predictor = predictors::SimpleLinear<ValueT_, 2>
>
class ValueCompressor {
public:
    using ValueT = ValueT_;

    ValueCompressor() = default;
    ValueCompressor(Predictor predictor)
      : predictor(std::move(predictor)) {}

    /// Store a value into the value stream.
    template <typename Encoder>
    void write(const ValueT& value, Encoder& encoder) {

        const auto prediction = predictor->predict_and_feed(value);

        if (prediction == value) {
            encoder->predictor_hit();
        } else {
            const auto [absError, predictionHigh] = abs_diff(prediction, value);
            encoder->predictor_miss(predictionHigh, absError, errorEncoder);
        }
    }

private:
    Predictor predictor;
    AdaptiveExpGolombEncoder<std::make_unsigned_t<ValueT_>> errorEncoder;
};

}
