#pragma once

#include <cstdint>
#include <limits>
#include <cassert>

namespace tablog::detail {

/// Takes stream items on input, outputs bytes through OutputF
/// This class can't encode complete predictor comparisons, because there may be several
/// streaks interleaved
template <typename OutputF>
class StreamEncoder {
public:

    StreamEncoder(OutputF output)
        : output(std::move(output)) {}

    constexpr uint_fast8_t get_max_hit_streak_length() const {
        return 7;
    }

    void header(uint_fast8_t version, uint_fast8_t fieldCount) {
        number(version);
        number(fieldCount);
    }

    /// Encode a predictor hit streak.
    /// Streak length must be greater than 0 and less or equal than maxHitStreakLength.
    void predictor_hit_streak(uint_fast8_t streakLength) {
        assert(streakLength > 0);
        assert(streakLength <= get_max_hit_streak_length());

        output_nibble(predictorHit | streakLength);
    }

    /// Encode a predictor miss, with given sign and absolute value.
    /// Absolute value here must already be shifted by tolerance -- this function
    /// expects minimal values of absErrorToEncode to be 1
    /// Sign and value are separate, because
    ///  - Values near zero are typically removed for tolerance, requiring abs value anyway
    ///  - We avoid possible overflows with large unsigned types
    template <typename T>
    void predictor_miss(bool predictionHigh, T absErrorToEncode) {
        assert(absErrorToEncode > 0);

        absErrorToEncode -= 1;

        uint_fast8_t encoded = predictorMiss;
        encoded |= static_cast<uint_fast8_t>(predictionHigh) << predictorHighFlagShift;
        encoded |= absErrorToEncode & predictorMissValueMask;
        absErrorToEncode >> predictorMissValueBits;
        encoded |= (absErrorToEncode == 0) << predictorMissFinalFlagShift;
        output_nibble(encoded);

        if (absErrorToEncode > 0)
            number(absErrorToEncode);
    }

    /// Encode end of stream marker.
    void end_of_stream() {
        // End of stream is encoded by two end of stream nibbles (hit streak of length 0)
        // and then padded to full buffer size.

        output_nibble(predictorHit);
        output_nibble(predictorHit);
        while (bufferBitsUsed != 0)
            output_nibble(predictorHit);
    }

    /// Encode a number into nibbles with continuation bits
    template <typename T>
    void number(T number)
    {
        do {
            uint_fast8_t encoded  = (number & numberBlockMask);
            number >>= numberBlockBits;
            encoded |= (static_cast<uint_fast8_t>(number == 0) << numberFinalFlagShift);
            output_nibble(encoded);
        } while (number != 0);
    }

private:
    void output_nibble(uint_fast8_t nibble) {
        buffer |= nibble << bufferBitsUsed;
        bufferBitsUsed += nibbleBits;
        if (bufferBitsUsed == bufferSizeBits)
            flush_buffer();
    }

    void flush_buffer() {
        output(buffer);
        bufferBitsUsed = 0;
    }

    static constexpr uint_fast8_t nibbleBits = 4;

    static constexpr uint_fast8_t predictorHit = 0x8;

    static constexpr uint_fast8_t predictorMiss = 0x0;
    static constexpr uint_fast8_t predictorHighFlagShift = 2;
    static constexpr uint_fast8_t predictorMissFinalFlagShift = 1;
    static constexpr uint_fast8_t predictorMissValueBits = 1;
    static constexpr uint_fast8_t predictorMissValueMask = (1 << predictorMissValueBits) - 1;

    static constexpr uint_fast8_t numberFinalFlagShift = 3;
    static constexpr uint_fast8_t numberBlockBits = 3;
    static constexpr uint_fast8_t numberBlockMask = (1 << numberBlockBits) - 1;


    OutputF output;
    uint_fast8_t buffer;
    uint_fast8_t bufferBitsUsed = 0;

    // Always flush every byte, avoids potential problems with endianness
    static constexpr uint_fast8_t bufferSizeBits = 8;
};

};
