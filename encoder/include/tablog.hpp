#pragma once

#include "write_helper.h"
#include "stream_encoder.h"
#include "value_compressor.h"
#include "details.h"

#include <tuple>
#include <cstdint>

namespace tablog {

namespace detail {

template <typename StreamEncoder, typename... ValueTs>
class TablogInternal {
public:
    static constexpr uint_fast16_t formatVersion = 1;

    /// Write the header
    template <typename... EncoderArgs>
    TablogInternal(EncoderArgs&&... encoderArgs)
      : encoder(std::forward(encoderArgs)...)
    {
        // Tablog header:
        encoder.header(formatVersion, sizeof...(ValueTs));
    }

    ~TablogInternal() {
        close();
    }

    /// Write values to the log.
    void write(const ValueTs&... values) {
        detail::WriteHelper<
            0,
            decltype(valueCompressors),
            decltype(encoder),
            ValueTs...
        >::write(valueCompressors, encoder, values...);
    }

    void close() {
        if (closed)
            return;

        encoder.end_of_stream();
        closed = true;
    }

private:
    bool closed;
    detail::StreamEncoder<OutputF> encoder;
    std::tuple<detail::ValueCompressor<ValueTs>...> valueCompressors;
};

namespace tablog {

/// This is the main class of the tablog library, with compile time interface and
/// default encoder.
template <typename OutputF, typename... ValueTs>
using Tablog = detail::TablogInternal<detail::StreamEncoder<OutputF>, ValueTs...>;

}
