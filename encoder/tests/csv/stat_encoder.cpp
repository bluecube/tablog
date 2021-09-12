#include "stat_encoder.hpp"

namespace tablog::test {

StatEncoder::StatEncoder(std::ostream& stream)
  : stream(stream),
    streamEncoder{StreamOutF{*this}}
{

    for (size_t i = 0; i < maxHitStreakLength; ++i)
        streakCounts[i] = 0;

    for (size_t i = 0; i < maxMissDistance; ++i) {
        positiveCounts[i] = 0;
        negativeCounts[i] = 0;
    }

    encodedByteCount = 0;
}

void StatEncoder::header(uint_fast8_t version, uint_fast8_t fieldCount) {
    streamEncoder.header(version, fieldCount);
}

void StatEncoder::end_of_stream() {
    streamEncoder.end_of_stream();
    for (ssize_t i = 0; i < static_cast<ssize_t>(maxMissDistance); ++i) {
        auto index = static_cast<ssize_t>(maxMissDistance) - 1 - i;
        auto distance = -static_cast<ssize_t>(maxMissDistance) + i;

        stream << distance << ": " << negativeCounts[index] << "\n";
    }

    stream << "0:";
    for (size_t i = 0; i < maxHitStreakLength; ++i)
        stream << " x" << (i + 1) << ": " << streakCounts[i];
    stream << "\n";

    for (size_t i = 0; i < maxMissDistance; ++i)
        stream << (i + 1) << ": " << positiveCounts[i] << "\n";

    stream << "\n";
    stream << "Encoded size: " << encodedByteCount << " B\n";
}

void StatEncoder::predictor_hit_streak(uint_fast8_t streakLength) {
    streamEncoder.predictor_hit_streak(streakLength);

    streakCounts[streakLength - 1]++;
}

void StatEncoder::predictor_miss(bool predictionHigh, uint64_t absErrorToEncode) {
    streamEncoder.predictor_miss(predictionHigh, absErrorToEncode);

    if (predictionHigh) {
        positiveCounts[absErrorToEncode - 1]++;
    } else {
        negativeCounts[absErrorToEncode - 1]++;
    }
}

}