#pragma once

#include "dynamic/encoder.hpp"

#include <cstdint>
#include <limits>
#include <cassert>
#include <iostream>

#include "stream_encoder.hpp"

namespace tablog::test {

/// Takes stream items on input, outputs bytes through OutputF
class StatEncoder : public dynamic::EncoderInterface {
public:
    StatEncoder(std::ostream& stream);

    /* This is to make the compression comparable to stream encoder, in fact we could use uint64 max.*/
    static constexpr uint_fast8_t maxHitStreakLength = 7;

    uint_fast8_t get_max_hit_streak_length() const override { return maxHitStreakLength; }
    void header(uint_fast8_t version, uint_fast8_t fieldCount) override;
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) override;
    void predictor_hit_streak(uint_fast8_t streakLength) override;
    void predictor_miss(bool predictionHigh, uint64_t absErrorToEncode) override;
    void end_of_stream() override;

private:
    struct StreamOutF {
        void operator()(uint8_t) {
            se.encodedByteCount++;
        }

        StatEncoder& se;
    };

    std::ostream& stream;

    static constexpr size_t maxMissDistance = 20;

    size_t streakCounts[maxHitStreakLength];
    size_t positiveCounts[maxMissDistance];
    size_t negativeCounts[maxMissDistance];
    size_t encodedByteCount;

    detail::StreamEncoder<StreamOutF> streamEncoder;
};

};
