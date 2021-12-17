#pragma once

#include "util/misc.hpp"
#include "stream_encoder_bits.hpp"
#include "predictors.hpp"

#include <cstdint>

namespace tablog::detail {

/// Class that compresses a stream of values.
template <typename ValueT_>
class ValueCompressor {
public:
    using ValueT = ValueT_;

    /// Store a value into the value stream.
    template <typename Encoder>
    void write(const ValueT& value, Encoder& encoder) {

        const auto prediction = predictor.predict_and_feed(value);

        if (prediction == value) {
            encoder.predictor_hit();
        } else {
            const auto [absError, predictionHigh] = abs_diff(prediction, value);
            encoder.predictor_miss(predictionHigh, absError, errorEncoder);
        }
    }

private:
    predictors::Linear12Adapt<ValueT> predictor;
    AdaptiveExpGolombEncoder<std::make_unsigned_t<ValueT_>> errorEncoder;
};

}
