#include "stream_encoder.hpp"
#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"

#include <iostream>
#include <cstdlib>
#include <string>
#include <string_view>
#include <charconv>
#include <optional>

using namespace tablog;

#define TYPED_CALL(fun, type, ...) \
    do { \
        if      (type == "u8")  fun<uint8_t> (__VA_ARGS__); \
        else if (type == "s8")  fun<int8_t>  (__VA_ARGS__); \
        else if (type == "u16") fun<uint16_t>(__VA_ARGS__); \
        else if (type == "s16") fun<int16_t> (__VA_ARGS__); \
        else if (type == "u32") fun<uint32_t>(__VA_ARGS__); \
        else if (type == "s32") fun<int32_t> (__VA_ARGS__); \
        else if (type == "u64") fun<uint64_t>(__VA_ARGS__); \
        else if (type == "s64") fun<int64_t> (__VA_ARGS__); \
        else throw std::runtime_error("Unsupported field type for typed function call"); \
    } while(0)

template <typename T>
T parse_num(std::string_view s) {
    T value;
    const auto end = s.data() + s.size();
    const auto convResult = std::from_chars(s.data(), end, value, 10);
    if (convResult.ptr != end)
        throw std::runtime_error("Number parsing failed");

    return value;
}

/// Return a view of a line until the next coma separator or nullopt for the last item
std::optional<std::string_view> next_token(std::string_view& line) {
    if (!line.size())
        return std::nullopt;

    const auto pos = line.find(',');
    if (pos == line.npos) {
        auto ret = line;
        line = std::string_view();
        return ret;
    }

    auto ret = line.substr(0, pos);
    line = line.substr(pos + 1);
    return ret;
}

template <typename T,  typename BW>
void write_bits(BW& bitWriter, std::string_view args) {
    if constexpr (!std::is_signed_v<T>) {
        while (1) {
            const auto tok = next_token(args);

            if (!tok.has_value())
                return;

            const auto value = parse_num<T>(tok.value());
            const auto bitCount = parse_num<T>(next_token(args).value());

            bitWriter.write(value, bitCount);
        }
    }
}

template <typename T,  typename BW>
void elias_gamma(BW& bitWriter, std::string_view args) {
    const T value = parse_num<T>(next_token(args).value());

    if (value < 0)
        throw std::runtime_error("Elias Gamma can't encode negative numbers");

    const auto unsignedValue = static_cast<std::make_unsigned_t<T>>(value);

    tablog::detail::elias_gamma(unsignedValue, bitWriter);
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
        else if (func == "string")
            string(bitWriter, rest);
        else {
            const auto type = next_token(rest).value();

            if (func == "write_bits")
                TYPED_CALL(write_bits, type, bitWriter, rest);
            else if (func == "elias_gamma")
                TYPED_CALL(elias_gamma, type, bitWriter, rest);
            else
                throw std::runtime_error("Unknown func");
        }

        bitWriter.flush();

        std::cout << output.size();
        std::cout << '\n';
        std::cout << output;
    }
}
