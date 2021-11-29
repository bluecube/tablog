#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <iostream>

#include "util/misc.hpp"

namespace tablog::detail {

/// Encode a positive number using Elias gamma encoding,
/// offset by 1, so that we can store zero
template <typename T, typename BW>
inline void elias_gamma(T n, BW& bitWriter) {
    static_assert(!std::is_signed_v<T>);

    ++n; // Add the offset to allow storing a zero

    const auto logN = int_log2(n); // TODO: use std::bit_width(n) - 1, once we bump to c++20
    bitWriter.write(T(0), logN);
    bitWriter.write_bit(1);
    bitWriter.write(n, logN); // skips the most significant bit in `n`

    // Since we write numbers with least significant bit first, the
    // stored number is "deformed".
}

/// Encode the number using adaptive exp-golomb coding.
/// This function can encode zero values.
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
/// \arg streamState An auxiliary state variable that describes the typical
///      range of the encoded values and adapts to the input values.
template <typename T, typename BW>
void adaptive_exp_golomb(T n, uint8_t& streamState, BW& bitWriter) {
    static_assert(!std::is_signed_v<T>);

    constexpr uint_fast8_t streamStateShift = 2;

    const auto k = streamState >> streamStateShift;
    const auto p = n >> k;

    elias_gamma(p, bitWriter); // Quotient; elias gamma can't encode zeros, therefore + 1
    bitWriter.write(n, k); // Remainder (masking upper bits is performed by bit writer)

    streamState += p - 1; // Update the adaptive shift value
}

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

}
