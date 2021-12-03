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

    static constexpr uint_fast8_t maxHitStreakLength = 128;

    void header(uint_fast8_t version, uint_fast8_t fieldCount) override;
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) override;
    void predictor_hit() override;
    void predictor_miss(
        bool predictionHigh,
        uint64_t absErrorToEncode,
        detail::AdaptiveExpGolombEncoder<uint64_t>& errorEncoder
    ) override;
    void end_of_stream() override;

private:
    struct StreamOutF {
        void operator()(uint8_t) {
            se.encodedByteCount++;
        }

        StatEncoder& se;
    };

    std::ostream& stream;

    static constexpr size_t maxMissDistance = 50;

    size_t hitCount;
    size_t positiveCounts[maxMissDistance];
    size_t negativeCounts[maxMissDistance];
    size_t encodedByteCount;

    detail::StreamEncoder<StreamOutF> streamEncoder;
};

};
