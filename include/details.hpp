#pragma once

namespace tablog::detail {

template <std::size_t I, typename CompressorsTupleT, typename EncoderT, typename T1, typename... Ts>
struct WriteHelper {
    void write(
        CompressorsTupleT& compressors, EncoderT& encoder,
        const T1& first, const Ts&... others
    )
    {
        std::get<I>(compressors).write(first, encoder);
        WriteHelper<I + 1>{}.(compressors, encoder, others...);
    }
};

template <std::size_t I, typename CompressorsTupleT, typename EncoderT>
struct WriteHelper<I, CompressorsTupleT, EncoderT> {
    void write(CompressorsTuppleT& compressors, EncoderT& encoder) {}
};

}
