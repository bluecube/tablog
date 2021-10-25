#pragma once

#include "util/misc.hpp"
#include "util/bit_writer.hpp"

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
        elias_gamma(version + 1u);
        elias_gamma(fieldCount);
    }

    /// Output field header. Field headers must directly follow header, with the
    /// same count as fieldCount in the header.
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) {
        string(fieldName);
        uint_fast8_t typeExponent = small_int_log2(typeSize);
        output.write_bit(signedType);
        output.write(typeExponent, 2);
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
    void predictor_miss(bool predictionHigh, T absErrorToEncode, uint8_t& streamState) {
        assert(absErrorToEncode > 0);

        output.write_bit(0);
        output.write_bit(predictionHigh);

        adaptive_exp_golomb(absErrorToEncode - 1, streamState);
    }

    void end_of_stream() {
        // TODO: Implement
    }

private:
    static constexpr uint_fast8_t streamStateShift = 2;

    /// Encode the number using adaptive exp-golomb coding.
    /// This function can encode zero values.
    ///
    /// Loosely based on
    /// Henrique S. Malvar: Adaptive Run-Length / Golomb-Rice Encoding of
    ///     Quantized Generalized Gaussian Sources with Unknown Statistics
    /// https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/Malvar_DCC06.pdf
    ///
    /// We use the EXP variant, because for anomalous inputs the non-exp version
    /// could fail catastrophically.
    /// Eg. 64 bit value, that has low values and low prediction errors, suddenly
    /// jumping to INT64_MAX. This would cause the unary part of golomb coding to
    /// explode to 2**64 bits of length.
    template <typename T>
    void adaptive_exp_golomb(T n, uint8_t& streamState) {
        const auto k = streamState >> streamStateShift;
        const auto p = n >> k;

        elias_gamma(p + 1); // Quotient; elias gamma can't encode zeros, therefore + 1
        output.write(n, k); // Remainder (masking upper bits is performed by bit writer)

        streamState += p - 1; // Update the adaptive shift value
    }

    template <typename T>
    void elias_gamma(T n) {
        static_assert(!std::is_signed_v<T>);
        assert(n > 0);

        auto logN = int_log2(n); // TODO: use std::bit_width(n) - 1, once we bump to c++20
        output.write(T(0), logN);
        output.write(n, logN - 1);
    }

    /// Encode the string into the output as a sequence
    /// Null string is equivalent to empty string.
    void string(const char* str) {
        (void)str;
        /*if (str) {
            ValueCompressor<uint8_t, predictors::Constant<uint8_t, 'e'>> compressor;
            number(strlen(str));
            while (*fieldName)
                compressor.write(*(fieldName++), *this);
        }
        else
            number((uint8_fast_t)0);*/
    }

    util::BitWriter<OutputF> output;
};

}
