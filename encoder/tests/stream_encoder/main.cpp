#include "../common.hpp"

#include "stream_encoder.hpp"
#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"
#include "predictors.hpp"
#include "framing.hpp"

#include <iostream>
#include <string>
#include <string_view>

using namespace tablog;


template <typename T,  typename BW>
void write_bits(BW& bitWriter, std::string_view args) {
    while (1) {
        const auto tok = next_token(args);

        if (!tok.has_value())
            return;

        const auto value = parse_num<T>(tok.value());
        const auto bitCount = parse_num<uint8_t>(next_token(args).value());

        bitWriter.write(value, bitCount);
    }
}

template <typename T,  typename BW>
void elias_gamma(BW& bitWriter, std::string_view args) {
    const auto value = parse_num<T>(next_token(args).value());
    tablog::detail::elias_gamma(value, bitWriter);
}

template <typename T,  typename BW>
void adaptive_exp_golomb(BW& bitWriter, std::string_view args) {
    detail::AdaptiveExpGolombEncoder<T> encoder;
    while (1) {
        const auto tok = next_token(args);

        if (!tok.has_value())
            return;

        const auto value = parse_num<T>(tok.value());
        encoder.encode(value, bitWriter);
    }
}

template <typename BW>
void str(BW& bitWriter, std::string_view args) {
    tablog::string::compress_string(args, bitWriter);
}

/// Test pattern consisting of bytes with increasing value
void byte_pattern(std::string& output, std::string_view args) {
    const auto count = parse_num<uint32_t>(args);
    for (uint32_t i = 0; i < count; ++i)
        output.push_back(i & 0xff);
}

/// Test pattern consisting of repeated zeroes and ones
template <typename BW>
void bit_pattern(BW& bitWriter, std::string_view args) {
    const auto count = parse_num<uint32_t>(next_token(args).value());
    for (uint32_t i = 0; i < count; ++i)
        bitWriter.write_bit(i & 1);
}

/// Test pattern consisting of non-byte aligned incrementing numbers
template <typename BW>
void bit_pattern2(BW& bitWriter, std::string_view args) {
    const auto count = parse_num<uint32_t>(next_token(args).value());
    for (uint32_t i = 0; i < count; ++i)
        bitWriter.write(i & 0x1f, 5);
}

void framing(std::string& output, std::string_view args) {
    tablog::detail::Framing f([&output](uint8_t c) mutable { output.push_back(c); });
    for (auto c: args) {
        if (c == '1')
            f.start();
        else if (c == '2')
            f.end();
        else
            f(c);
    }
}

template <typename T, typename BW>
void encode_type(BW& bitWriter) {
    tablog::detail::encode_type<T>(bitWriter);
}

template <typename T, typename BW>
void type_info(BW& bitWriter) {
    bitWriter.write_bit(std::is_signed_v<T>);
    tablog::detail::elias_gamma(
        static_cast<std::make_unsigned_t<T>>(sizeof(T)),
        bitWriter
    );
    tablog::detail::elias_gamma(
        static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::digits),
        bitWriter
    );
    const auto min = std::numeric_limits<T>::min();
    if (min < 0)
        tablog::detail::elias_gamma(
            1u + static_cast<std::make_unsigned_t<T>>(-(min + 1)),
            bitWriter
        );
    else
        tablog::detail::elias_gamma(
            static_cast<std::make_unsigned_t<T>>(min),
            bitWriter
        );
    tablog::detail::elias_gamma(
        static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::max()),
        bitWriter
    );
}

template <typename PredictorT, typename T, typename BW>
void predictor_internal(BW& bitWriter, std::string_view args) {
    PredictorT predictor;
    while (const auto tok = next_token(args)) {
        const auto value = parse_num<T>(tok.value());
        const auto prediction = predictor.predict_and_feed(value);
        const auto unsignedPrediction = static_cast<std::make_unsigned_t<T>>(prediction);
        bitWriter.write(unsignedPrediction);
    }
}

template <typename T,  typename BW>
void linear12adapt_predictor(BW& bitWriter, std::string_view args) {
    predictor_internal<tablog::predictors::Linear12Adapt<T>, T, BW>(bitWriter, args);
}

int main() {
    std::string lineStr;
    while(std::getline(std::cin, lineStr)) {
        auto rest = std::string_view(lineStr);

        std::string output;

        const auto func = next_token(rest).value();

        if (func == "byte_pattern")
            byte_pattern(output, rest);
        else if (func == "framing")
            framing(output, rest);
        else {
            tablog::util::BitWriter bitWriter{[&output](uint8_t c) { output.push_back(c); }};

            if (func == "bit_pattern")
                bit_pattern(bitWriter, rest);
            else if (func == "bit_pattern2")
                bit_pattern2(bitWriter, rest);
            else if (func == "string")
                str(bitWriter, rest);
            else {
                const auto type = next_token(rest).value();

                if (func == "write_bits")
                    TYPED_CALL_UNSIGNED(write_bits, type, bitWriter, rest);
                else if (func == "elias_gamma")
                    TYPED_CALL_UNSIGNED(elias_gamma, type, bitWriter, rest);
                else if (func == "adaptive_exp_golomb")
                    TYPED_CALL_UNSIGNED(adaptive_exp_golomb, type, bitWriter, rest);
                else if (func == "encode_type")
                    TYPED_CALL(encode_type, type, bitWriter);
                else if (func == "type_info")
                    TYPED_CALL(type_info, type, bitWriter);
                else if (func == "linear12adapt_predictor")
                    TYPED_CALL(linear12adapt_predictor, type, bitWriter, rest);
                else
                    throw std::runtime_error("Unknown func"); // GCOV_EXCL_LINE
            }

            bitWriter.end();
        }

        std::cout << output.size();
        std::cout << '\n';
        std::cout << output;
    }
}
