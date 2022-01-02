#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <utility>
#include <cassert>

namespace tablog::util {

/// Helper class that allows writing unaligned bit data and passes them to the
/// output byte by byte.
template <typename OutputF>
class BitWriter {
private:
    using BufferT = uint_fast8_t;
public:
    BitWriter(OutputF output)
        : output(std::move(output)) {}

    /// Write bitCount least significant bits of data into the output.
    template <typename T>
    void write(T data, uint_fast8_t bitCount = std::numeric_limits<T>::digits) noexcept {
        static_assert(std::is_integral_v<T>, "Only integral types can be written");
        static_assert(!std::is_signed_v<T>, "Only unsigned types can be written");
        assert(bitCount <= std::numeric_limits<T>::digits);

        if (bitCount + bufferUsed < outputBitSize) {
            // Not enough data to output a byte
            buffer |= data << bufferUsed;
            bufferUsed += bitCount;
        } else {
            // First use beginning of the data to fill the rest of the buffer
            buffer |= (static_cast<BufferT>(data) << bufferUsed) & outputMask;
            output(buffer);
            auto shift = outputBitSize - bufferUsed;
            data >>= shift;
            bitCount -= shift;

            // Then output all the full bytes in the data input
            while (bitCount >= outputBitSize) {
                output(data & outputMask);
                data >>= outputBitSize;
                bitCount -= outputBitSize;
            }

            // Set the buffer to the remainder.
            buffer = data & ((1 << bitCount) - 1);
            bufferUsed = bitCount;
        }

        assert(bufferUsed < outputBitSize);
        buffer &= ((1 << bufferUsed) - 1);
        assert(buffer >> bufferUsed == 0);
    }

    void write_bit(uint_fast8_t b) {
        write(b, 1);
    }

    /// End the bit stream by writing a single 1 bit and then padding with
    /// zero bits to whole byte.
    void end() {
        this->write_bit(1);
        this->flush();
    }

    /// Pad the last unwritten byte with zeros and output it.
    /// This is only intended for testing
    void flush() {
        if (bufferUsed == 0)
            return;

        output(buffer);
        buffer = 0;
        bufferUsed = 0;
    }

    OutputF output;

private:
    static constexpr uint_fast8_t outputBitSize = 8;
    static constexpr uint_fast8_t outputMask = 0xff;

    BufferT buffer = 0;
    uint_fast8_t bufferUsed = 0;
};

}
