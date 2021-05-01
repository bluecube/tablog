#pragma once

#include "write_helper.h"
#include "stream_encoder.h"
#include "value_compressor.h"

#include <tuple>

namespace tablog {

template <typename OutputF, typename... ValueTs>
class Tablog {
public:
    static constexpr uint_fast16_t formatVersion = 1;

    /// Write the header
    Tablog(OutputF outputFunction)
      : encoder(std::move(outputFunction))
    {
        // Tablog header:
        encoder.number(formatVersion);
        encoder.number(sizeof...(ValueTs));
    }

    ~Tablog() {
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

}
