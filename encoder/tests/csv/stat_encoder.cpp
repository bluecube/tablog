#include "stat_encoder.hpp"

namespace tablog::test {

StatEncoder::StatEncoder(std::ostream& stream)
  : stream(stream),
    streamEncoder{StreamOutF{*this}}
{

    for (size_t i = 0; i < maxMissDistance; ++i) {
        positiveCounts[i] = 0;
        negativeCounts[i] = 0;
    }

    encodedByteCount = 0;
}

void StatEncoder::header(uint_fast8_t version, uint_fast8_t fieldCount) {
    streamEncoder.header(version, fieldCount);
}

void StatEncoder::field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) {
    streamEncoder.field_header(fieldName, signedType, typeSize);
}

void StatEncoder::end_of_stream() {
    //streamEncoder.end_of_stream();
    for (ssize_t i = 0; i < static_cast<ssize_t>(maxMissDistance); ++i) {
        auto index = static_cast<ssize_t>(maxMissDistance) - 1 - i;
        auto distance = -static_cast<ssize_t>(maxMissDistance) + i;

        stream << distance << ": " << negativeCounts[index] << "\n";
    }

    stream << "0:" << hitCount << "\n";

    for (size_t i = 0; i < maxMissDistance; ++i)
        stream << (i + 1) << ": " << positiveCounts[i] << "\n";

    stream << "\n";
    stream << "Encoded size: " << encodedByteCount << " B\n";
}

void StatEncoder::predictor_hit() {
    streamEncoder.predictor_hit();
    ++hitCount;
}

void StatEncoder::predictor_miss(
    bool predictionHigh,
    uint64_t absErrorToEncode,
    detail::AdaptiveExpGolombEncoder<uint64_t>& errorEncoder
) {
    streamEncoder.predictor_miss(predictionHigh, absErrorToEncode, errorEncoder);

    if (absErrorToEncode > maxMissDistance)
        absErrorToEncode = maxMissDistance;

    if (predictionHigh) {
        positiveCounts[absErrorToEncode - 1]++;
    } else {
        negativeCounts[absErrorToEncode - 1]++;
    }
}

}
