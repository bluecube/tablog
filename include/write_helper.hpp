#pragma once

#include <tuple>
#include <cstddef>

namespace tablog::detail {

template <std::size_t I, typename CompressorsTupleT, typename EncoderT, typename... Ts>
struct WriteHelper;

template <std::size_t I, typename CompressorsTupleT, typename EncoderT, typename T1, typename... Ts>
struct WriteHelper<I, CompressorsTupleT, EncoderT, T1, Ts...> {
    static void write(
        CompressorsTupleT& compressors, EncoderT& encoder,
        const T1& first, const Ts&... others
    )
    {
        std::get<I>(compressors).write(first, encoder);
        WriteHelper<I + 1, CompressorsTupleT, EncoderT, Ts...>::write(
            compressors, encoder, others...
        );
    }
};

template <std::size_t I, typename CompressorsTupleT, typename EncoderT>
struct WriteHelper<I, CompressorsTupleT, EncoderT> {
    static void write(CompressorsTupleT& compressors, EncoderT& encoder) {}
};

}
