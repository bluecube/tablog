#pragma once

#include "column_compressor.hpp"
#include "framing.hpp"
#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"

#include <tuple>
#include <cstdint>
#include <utility>
#include <optional>

namespace tablog {

/// Main class of Tablog.
/// Encodes rows of values into a compressed sequence of blocks.
///
/// OutputF is a (duck typed) subclass of BaseOutptutAdapter,
/// determining what to do with the compressed data.
/// ValueTs are types of column data.
template <typename OutputF, typename... ValueTs>
class Tablog {
public:
    static constexpr uint_fast16_t outputFormatVersion = 0;

    template <typename... OutputFArgs>
    Tablog(OutputFArgs&&... outputFArgs)
      : bitWriter(detail::Framing<OutputF>(OutputF(std::forward<OutputFArgs>(outputFArgs)...))) {}

    ~Tablog() {
        if (columnCompressors.has_value())
            end_block();
    }

    void write_row(const ValueTs&... values) {
        if (!columnCompressors.has_value())
            start_block();
        std::apply(
            [&](auto&... column) {
                (column.write_value(values, bitWriter), ...);
            },
            *columnCompressors
        );
    }

    /// End the current block of the compressed data.
    /// This inserts a sequence point to the output and repeats all headers.
    /// This allows re-synchronization of a decoder state.
    /// This method is safe to call at any time.
    /// When a block has not been started yet, outputs an empty block (starts a block and immediately ends it).
    void end_block() {
        if (!columnCompressors.has_value())
            start_block();
        bitWriter.end_bit_stream();
        bitWriter.output.write_end_mark();
        columnCompressors.reset();
    }

private:
    void start_block() {
        columnCompressors.emplace();
        bitWriter.output.write_start_mark();
        detail::elias_gamma(outputFormatVersion, bitWriter);
        detail::elias_gamma(sizeof...(ValueTs) - 1u, bitWriter);
        std::apply(
            [&](auto&... column) {
                (column.write_header(bitWriter), ...);
            },
            *columnCompressors
        );
    }

    util::BitWriter<detail::Framing<OutputF>> bitWriter;
    std::optional<std::tuple<detail::ColumnCompressor<ValueTs>...>> columnCompressors;
};

}
