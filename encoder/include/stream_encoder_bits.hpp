#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

#include "util/misc.hpp"

namespace tablog::detail {

/// Encode a positive number using Elias gamma encoding,
/// offset by 1, so that we can store zero
template <typename T, typename BW>
inline void elias_gamma(T n, BW& bitWriter) {
    static_assert(!std::is_signed_v<T>);

    ++n; // Add the offset to allow storing a zero

    // TODO: Performance: If we allow working in larger types sometime in the future, this branch can be avoided.
    const auto logN = [&]() {
        if (n != 0)
            return int_log2(n); // TODO: use std::bit_width(n) - 1, once we bump to c++20
        else
            // If the number had an overflow, we know that the logarithm depends
            // only on the number of digits in the type.
            return static_cast<uint_fast8_t>(std::numeric_limits<T>::digits);
    }();
    bitWriter.write(T(0), logN);
    bitWriter.write_bit(1);
    bitWriter.write(n, logN); // skips the most significant bit in `n`

    // Since we write numbers with least significant bit first, the
    // stored number is "deformed".
}

/// Encode the number using adaptive exp-golomb coding.
/// The class holds state necessary for the encoding.
/// Can encode zero values.
///
/// Loosely based on
/// Henrique S. Malvar: Adaptive Run-Length / Golomb-Rice Encoding of
///     Quantized Generalized Gaussian Sources with Unknown Statistics
/// https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/Malvar_DCC06.pdf
///
/// We use the EXP variant, because for anomalous inputs the non-exp version
/// could fail catastrophically.
/// Eg. 64 bit value, that has low values and low prediction errors, suddenly
/// jumping to INT64_MAX. This would cause the unary part of golomb coding to
/// explode to 2**64 bits of length.
///
/// Unlike the article, we also only ever update the state variable by +-1.
/// This avoids problems with isolated misspredictions, where the large error would
/// skew the state towards too large values of k.
template <typename T>
class AdaptiveExpGolombEncoder {
public:
    template <typename BW>
    void encode(T n, BW& bitWriter) {
        const T k = state >> stateShift;
        const T p = n >> k;

        elias_gamma(p, bitWriter); // Quotient; elias gamma can't encode zeros, therefore + 1
        bitWriter.write(n, k); // Remainder (masking upper bits is performed by bit writer)

        // Update the adaptive shift value
        if (p == 0 && state > 0)
            state -= 1;
        else if (p > 1 && state < maxState)
            state += 1;
    }

    /// For debugging: Return the internal state of the encoder
    uint8_t get_state() const { return state; }
protected:
    /// Determines how fast the adaptation reacts to value changes.
    static constexpr uint_fast8_t stateShift = 2;

    /// State for the encoder, initialized to a very rough estimate of noise level
    uint8_t state = (sizeof(T)) << stateShift;

    static_assert(!std::is_signed_v<T>);
    static_assert(
        (std::numeric_limits<decltype(state)>::max() >> stateShift)
        >=
        std::numeric_limits<T>::digits - 1u,
        "State after shift must be able to shift out all bits of T "
        "(this assures that state never overflows)"
    );

    /// Maximum value of state shift, makes sure that we don't over-shift more than
    /// the full range of T worth of bits.
    static constexpr uint_fast8_t maxState = (std::numeric_limits<T>::digits << stateShift) - 1;
};

/// Encode the string into the output as a sequence
/// Null string is equivalent to empty string.
template <typename BW>
inline void encode_string(const char* str, BW& bitWriter) {
    (void)str;
    (void)bitWriter;

    /*if (str) {
        ValueCompressor<uint8_t, predictors::Constant<uint8_t, 'e'>> compressor;
        number(strlen(str));
        while (*fieldName)
            compressor.write(*(fieldName++), *this);
    }
    else
        number((uint8_fast_t)0);*/
}

template <typename BW>
inline void encode_type(bool signedType, uint_fast8_t typeSize, BW& bitWriter) {
    uint_fast8_t typeExponent = small_int_log2(typeSize);
    bitWriter.write_bit(signedType);
    bitWriter.write(typeExponent, 2);
}

template <typename T, typename BW>
inline void encode_type(BW& bitWriter) {
    encode_type(std::is_signed_v<T>, sizeof(T), bitWriter);
}

}
