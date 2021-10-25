#include "json_encoder.hpp"

namespace tablog::test {

void JsonEncoder::header(uint_fast8_t version, uint_fast8_t fieldCount) {
    stream <<
        "{\n" <<
        "  \"version\": " << static_cast<uint64_t>(version) << ",\n" <<
        "  \"field_count\": " << static_cast<uint64_t>(fieldCount) << ",\n" <<
        "  \"field_descriptors\": [\n";
}

void JsonEncoder::field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) {
    stream <<
        "    {\n" <<
        "      \"name\": \"" << fieldName << ",\n" <<
        "      \"type\": \"" << (signedType ? "s" : "u") << typeSize << "\"\n" <<
        "    }";
}

void JsonEncoder::end_of_stream() {
    stream <<
        "  ]\n" <<
        "}\n";
}

void JsonEncoder::predictor_hit() {
    if (!firstRecord)
        finalize_field_headers();

    stream <<
        "    {\n" <<
        "      \"error\": 0,\n" <<
        "    }";
}

void JsonEncoder::predictor_miss(bool predictionHigh, uint64_t absErrorToEncode, uint8_t&) {
    if (!firstRecord)
        finalize_field_headers();

    stream <<
        "    {\n" <<
        "      \"error\": " << (predictionHigh ? "" : "-") << absErrorToEncode << ",\n" <<
        "    }";
}

void JsonEncoder::finalize_field_headers() {
    stream <<
    "  ],\n" <<
    "  \"records\": [\n";
    firstRecord = false;
}

}
