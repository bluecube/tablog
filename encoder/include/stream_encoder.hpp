#pragma once

#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"

#include <cstdint>
#include <limits>
#include <utility>
#include <cassert>

namespace tablog::detail {

/// Takes stream items on input, outputs bytes through OutputF
/// This class can't encode complete predictor comparisons, because there may be several
/// streaks interleaved
template <typename OutputF_>
class StreamEncoder {
public:
    using OutputF = OutputF_;

    StreamEncoder(OutputF output)
        : output(std::move(output)) {}

    void header(uint_fast8_t version, uint_fast8_t fieldCount) {
        assert(fieldCount > 0);
        elias_gamma(version, output);
        elias_gamma(fieldCount - 1u, output);
    }

    /// Output field header. Field headers must directly follow header, with the
    /// same count as fieldCount in the header.
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) {
        encode_string(fieldName, output);
        encode_type(signedType, typeSize, output);
    }

    /// Encode a predictor hit streak.
    /// Streak length must be greater than 0 and less or equal than maxHitStreakLength.
    void predictor_hit() {
        output.write_bit(1);
    }

    /// Encode a predictor miss, with given sign and absolute value.
    /// Absolute value here must already be shifted by tolerance -- this function
    /// expects minimal values of absErrorToEncode to be 1
    template <typename T>
    void predictor_miss(
        bool predictionHigh,
        T absErrorToEncode,
        AdaptiveExpGolombEncoder<std::make_unsigned_t<T>>& errorEncoder
    ) {
        assert(absErrorToEncode > 0);

        output.write_bit(0);
        output.write_bit(predictionHigh);

        errorEncoder.encode(absErrorToEncode - 1, output);
    }

    void end_of_stream() {
        // TODO: Implement
    }

private:

    util::BitWriter<OutputF> output;
};

}
