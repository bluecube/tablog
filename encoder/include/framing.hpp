#pragma once

#include <cstdint>
#include <utility>
#include <cassert>

namespace tablog::detail {

/// Wrapper for a byte output function that provides message framing.
/// The encoded stream never contains neither start nor end marks unless explicitly
/// written.
/// Each mark is encoded as 2 bytes, expands non-mark data by
/// 3/2**16 = ~4.5e-3% on average and by 50% worst case (Input matching /(T[l# ])*/).
template <typename OutputF>
class Framing {
public:
    Framing(OutputF output)
        : output(std::move(output)) {}

    /// Pass a byte to the output, possibly escape it
    void operator()(uint8_t byte) {
        if (!hadEscape) {
            if (byte == escapeByte)
                hadEscape = true;
            output(byte);
        } else {
            switch (byte) {
            case escapeByte:
                // Two escape characters in a row don't mean anything special,
                // and so are not a problem, but we need to keep the hadEscape flag.
                output(byte);
                // hadEscape remains true
                break;
            case startByte:
            case endByte:
            case doubleEscapeByte:
                // If escape + this character were interpreted somehow, then escape
                // the escape character using double-escape character.
                output(doubleEscapeByte);
                [[fallthrough]];
            default:
                // In all other cases just write the byte itself.
                output(byte);
                hadEscape = false;
            };
        }
    }

    /// Output start mark.
    void start() {
        output_escaped(startByte);
    }

    /// Output end mark.
    void end() {
        output_escaped(endByte);
    }

private:
    // Assignment of bytes for each individual role is arbitrary
    // (but needs to remain stable for stream format compatibility)
    static constexpr uint8_t escapeByte = 'T'; ///< All escape sequences start with T
    static constexpr uint8_t startByte = 'l'; ///< "Tl" starts the file
    static constexpr uint8_t endByte = '#'; ///< "T#" ends the file
    static constexpr uint8_t doubleEscapeByte = ' '; ///< "T x" is used to escape "Tx"

    /// Writes an escape byte and a given function byte to output.
    /// The argument must not be an escape character itself.
    void output_escaped(uint8_t byte) {
        assert(byte != escapeByte);
        output(escapeByte);
        output(byte);
        hadEscape = false; // Last written byte was not an escape
    }

    OutputF output;
    bool hadEscape = false;
};

}
