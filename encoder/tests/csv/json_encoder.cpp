#include "json_encoder.hpp"

namespace tablog::test {

void JsonEncoder::header(uint_fast8_t version, uint_fast8_t fieldCount) {
    stream <<
        "{ " <<
        "\"version\": " << static_cast<uint64_t>(version) << ", " <<
        "\"field_count\": " << static_cast<uint64_t>(fieldCount) << ", " <<
        "\"field_descriptors\": [ ";
}

void JsonEncoder::field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) {
    if (!firstFieldHeader)
        std::cout << ", ";
    firstFieldHeader = false;
    stream <<
        "{ " <<
        "\"name\": \"" << fieldName << "\", " <<
        "\"type\": \"" << (signedType ? "s" : "u") << static_cast<unsigned>(8 * typeSize) << "\" " <<
        "}";
}

void JsonEncoder::end_of_stream() {
    if (firstRecord)
        finalize_field_headers();

    stream << "\n{}\n";
}

void JsonEncoder::predictor_hit() {
    if (firstRecord)
        finalize_field_headers();

    stream << "\n{ \"error\": 0 }";
}

void JsonEncoder::predictor_miss(bool predictionHigh, uint64_t absErrorToEncode, uint8_t&) {
    if (firstRecord)
        finalize_field_headers();

    stream << "\n{ \"error\": " << (predictionHigh ? "" : "-") << absErrorToEncode << " }";
}

void JsonEncoder::finalize_field_headers() {
    stream << " ] }";
    firstRecord = false;
}

}
