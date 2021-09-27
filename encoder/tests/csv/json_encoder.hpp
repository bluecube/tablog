#pragma once

#include "dynamic/encoder.hpp"

#include <cstdint>
#include <limits>
#include <cassert>
#include <iostream>

namespace tablog::test {

/// Takes stream items on input, outputs bytes through OutputF
class JsonEncoder : public dynamic::EncoderInterface {
public:
    JsonEncoder(std::ostream& stream)
        : stream(stream) {}

    uint_fast8_t get_max_hit_streak_length() const override { return 7; /* This is to make the compression comparable to stream encoder, in fact we could use uint64 max.*/ }
    void header(uint_fast8_t version, uint_fast8_t fieldCount) override;
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) override;
    void predictor_hit_streak(uint_fast8_t streakLength) override;
    void predictor_miss(bool predictionHigh, uint64_t absErrorToEncode) override;
    void end_of_stream() override;

private:
    void finalize_field_headers();
    std::ostream& stream;
    bool firstRecord = true;
};

};
