#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <utility>
#include <array>
#include <vector>
#include <charconv>
#include <stdexcept>

template <typename T> static constexpr std::string_view type_name = "???";
template <> constexpr std::string_view type_name<uint8_t> = "u8";
template <> constexpr std::string_view type_name<int8_t> = "s8";
template <> constexpr std::string_view type_name<uint16_t> = "u16";
template <> constexpr std::string_view type_name<int16_t> = "s16";
template <> constexpr std::string_view type_name<uint32_t> = "u32";
template <> constexpr std::string_view type_name<int32_t> = "s32";
template <> constexpr std::string_view type_name<uint64_t> = "u64";
template <> constexpr std::string_view type_name<int64_t> = "s64";

template <typename... Ts>
static constexpr std::array<std::string_view, sizeof...(Ts)> type_names = {type_name<Ts>...};

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
static inline T parse_num(std::string_view s) {
    T value;
    const auto end = s.data() + s.size();
    const auto convResult = std::from_chars(s.data(), end, value, 10);
    if (convResult.ptr != end)
        throw std::runtime_error("Number parsing failed");

    return value;
}

/// Return a view of a line until the next coma separator or nullopt for the last item
inline std::optional<std::string_view> next_token(std::string_view& line) {
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

