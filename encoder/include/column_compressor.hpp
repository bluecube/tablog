#pragma once

#include "util/misc.hpp"
#include "stream_encoder_bits.hpp"
#include "predictors.hpp"
#include "string_compression.hpp"

#include <cstdint>
#include <iostream>

namespace tablog::detail {

/// Class that holds state relevant to compression of a single stream of values.
template <typename ValueT_>
class ColumnCompressor {
public:
    using ValueT = ValueT_;

    /// Store a value into the value stream.
    template <typename BitWriter>
    void write_value(const ValueT& value, BitWriter& bw) {
        const auto prediction = predictor.predict_and_feed(value);

        if (prediction == value) {
            bw.write_bit(1);
        } else {
            const auto [absError, predictionHigh] = abs_diff(prediction, value);
            assert(absError > 0);

            bw.write_bit(0);
            bw.write_bit(predictionHigh);
            errorEncoder.encode(absError - 1, bw);
        }
    }

    template <typename BitWriter>
    void write_header(BitWriter& bw) {
        std::cerr << "Encoding type: signed: " << std::is_signed_v<ValueT> << " sizeof: " << sizeof(ValueT) << "\n";
        encode_int_type(std::is_signed_v<ValueT>, sizeof(ValueT), bw);
    }

private:
    predictors::Linear12Adapt<ValueT> predictor;
    AdaptiveExpGolombEncoder<std::make_unsigned_t<ValueT_>> errorEncoder;
};

}
