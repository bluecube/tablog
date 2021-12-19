#include "../common.hpp"

#include "stream_encoder.hpp"
#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"
#include "predictors.hpp"

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
void string(BW& bitWriter, std::string_view args) {
    tablog::detail::encode_string(args.data(), bitWriter); // Assume that args is zero terminated
}

/// Test pattern consisting of bytes with increasing value
template <typename BW>
void byte_pattern(BW& bitWriter, std::string_view args) {
    const auto count = parse_num<uint32_t>(args);
    for (uint32_t i = 0; i < count; ++i)
        bitWriter.write(i & 0xff, 8);
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

template <typename T, typename BW>
void linear3_predictor(BW& bitWriter, std::string_view args) {
    predictor_internal<tablog::predictors::SimpleLinear<T, 3>, T, BW>(bitWriter, args);
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
        tablog::util::BitWriter bitWriter{[&output](uint8_t c) { output.push_back(c); }};

        const auto func = next_token(rest).value();

        if (func == "byte_pattern")
            byte_pattern(bitWriter, rest);
        else if (func == "bit_pattern")
            bit_pattern(bitWriter, rest);
        else if (func == "bit_pattern2")
            bit_pattern2(bitWriter, rest);
        else if (func == "string")
            string(bitWriter, rest);
        else {
            const auto type = next_token(rest).value();

            if (func == "write_bits")
                TYPED_CALL_UNSIGNED(write_bits, type, bitWriter, rest);
            else if (func == "elias_gamma")
                TYPED_CALL_UNSIGNED(elias_gamma, type, bitWriter, rest);
            else if (func == "adaptive_exp_golomb")
                TYPED_CALL_UNSIGNED(adaptive_exp_golomb, type, bitWriter, rest);
            else if (func == "linear3_predictor")
                TYPED_CALL(linear3_predictor, type, bitWriter, rest);
            else if (func == "linear12adapt_predictor")
                TYPED_CALL(linear12adapt_predictor, type, bitWriter, rest);
            else
                throw std::runtime_error("Unknown func");
        }

        bitWriter.flush();

        std::cout << output.size();
        std::cout << '\n';
        std::cout << output;
    }
}
