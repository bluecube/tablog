#pragma once

#include "stream_encoder.hpp"
#include "value_compressor.hpp"

#include <tuple>
#include <cstdint>
#include <utility>

namespace tablog {

template <typename OutputF, typename... ValueTs>
class Tablog {
public:
    static constexpr uint_fast16_t formatVersion = 0;

    /// Write the header
    template <typename... EncoderArgs>
    Tablog(EncoderArgs&&... encoderArgs)
      : encoder(std::forward<EncoderArgs>(encoderArgs)...)
    {
        encoder.header(formatVersion, sizeof...(ValueTs));
        (encoder.field_header(
            nullptr,
            std::is_signed_v<ValueTs>,
            sizeof(ValueTs)
        ), ...);
    }

    ~Tablog() {
        close();
    }

    /// Write values to the log.
    void write(const ValueTs&... values) {
        std::apply(
            [&](auto&... compressors) {
                (compressors.write(values, encoder), ...);
            },
            valueCompressors
        );
    }

    void close() {
        if (closed)
            return;

        encoder.end_of_stream();
        closed = true;
    }

private:
    bool closed = false;
    detail::StreamEncoder<OutputF> encoder;
    std::tuple<
        detail::ValueCompressor<
            ValueTs
        >...
    > valueCompressors;
};

}
