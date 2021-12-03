#pragma once

#include <cstdint>
#include <memory>

#include "stream_encoder_bits.hpp"

namespace tablog::dynamic {

class EncoderInterface;
template <typename T> class EncoderAdapter;

using Encoder = std::unique_ptr<EncoderInterface>;
template <typename T, typename... Args>
Encoder make_dynamic_encoder(Args&&... args) {
    return std::make_unique<EncoderAdapter<T>>(std::forward<Args>(args)...);
}

/// Takes stream items on input, outputs them somehow.
class EncoderInterface {
public:
    virtual ~EncoderInterface() {}

    /// Write the archive header
    virtual void header(uint_fast8_t version, uint_fast8_t fieldCount) = 0;

    /// Encode header of an individual field.
    virtual void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) = 0;

    /// Encode a predictor hit streak.
    /// Streak length must be greater than 0 and less or equal than maxHitStreakLength.
    virtual void predictor_hit() = 0;

    /// Encode a predictor miss, with given sign and absolute value.
    /// Absolute value here must already be shifted by tolerance -- this function
    /// expects minimal values of absErrorToEncode to be 1
    /// Sign and value are separate, because
    ///  - Values near zero might be removed for tolerance, requiring abs value anyway
    ///  - We avoid possible overflows with unsigned types
    virtual void predictor_miss(
        bool predictionHigh,
        uint64_t absErrorToEncode,
        detail::AdaptiveExpGolombEncoder<uint64_t>& errorEncoder
    ) = 0;

    /// Encode end of stream marker.
    virtual void end_of_stream() = 0;
};

template <typename T>
class EncoderAdapter: public EncoderInterface {
public:
    template <typename... Args>
    EncoderAdapter(Args&&... args) : encoder(std::forward<Args>(args)...) {}

    void header(uint_fast8_t version, uint_fast8_t fieldCount) override {
        encoder.header(version, fieldCount);
    }

    void field_header(const char* fieldName, bool signedType, uint_fast8_t typeSize) override {
        encoder.field_header(fieldName, signedType, typeSize);
    }

    void predictor_hit() override {
        encoder.predictor_hit();
    }

    void predictor_miss(
        bool predictionHigh,
        uint64_t absErrorToEncode,
        detail::AdaptiveExpGolombEncoder<uint64_t>& errorEncoder
    ) override {
        encoder.predictor_miss(predictionHigh, absErrorToEncode, errorEncoder);
    }

    /// Encode end of stream marker.
    void end_of_stream() {
        encoder.end_of_stream();
    }

private:
    T encoder;
};

};
