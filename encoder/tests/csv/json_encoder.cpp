#include "json_encoder.hpp"

namespace tablog::test {

void JsonEncoder::header(uint_fast8_t version, uint_fast8_t fieldCount) {
    stream <<
        "{\n" <<
        "  \"version\": " << static_cast<uint64_t>(version) << ",\n" <<
        "  \"field_count\": " << static_cast<uint64_t>(fieldCount) << ",\n" <<
        "  \"records\": [\n";
}

void JsonEncoder::end_of_stream() {
    stream <<
        "  ]\n" <<
        "}\n";
}

void JsonEncoder::predictor_hit_streak(uint_fast8_t streakLength) {
    if (!firstRecord)
        stream << ",\n";

    stream <<
        "    {\n" <<
        "      \"error\": 0,\n" <<
        "      \"streak_length\": " << static_cast<uint64_t>(streakLength) << "\n" <<
        "    }";
    firstRecord = false;
}

void JsonEncoder::predictor_miss(bool predictionHigh, uint64_t absErrorToEncode) {
    if (!firstRecord)
        stream << ",\n";

    stream <<
        "    {\n" <<
        "      \"error\": " << (predictionHigh ? "" : "-") << absErrorToEncode << ",\n" <<
        "      \"streak_length\": 1\n" <<
        "    }";
    firstRecord = false;
}

}
