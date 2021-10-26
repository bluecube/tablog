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

    void header(uint_fast8_t version, uint_fast8_t fieldCount) override;
    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) override;
    void predictor_hit() override;
    void predictor_miss(bool predictionHigh, uint64_t absErrorToEncode, uint8_t& encoderState) override;
    void end_of_stream() override;

private:
    void finalize_field_headers();
    std::ostream& stream;
    bool firstRecord = true;
    bool firstFieldHeader = true;
};

};
