#pragma once

#include "util/bit_writer.hpp"
#include "stream_encoder_bits.hpp"

#include <cstdint>
#include <string_view>
#include <algorithm>

namespace tablog::string {

namespace detail {

namespace trie {
struct Node {
    char c;
    uint8_t childIndex; // Linking to index where children of this node are stored
    uint8_t childCount: 4;
    uint8_t encodedLength: 4;
    uint16_t encodedValue;
};
}

// The following code was generated from the jupyter notebook string_predictors.ipynb
namespace trie {
static constexpr char firstLevelMin = ' ';
static constexpr char firstLevelMax = 'y';
static constexpr Node flattened[] = {
    {' ', 255, 0, 6, 37}, // ' '
    {'a', 255, 0, 8, 104}, // 'ea'
    {'d', 255, 0, 7, 10}, // 'ed'
    {'n', 53, 1, 7, 55}, // 'en'
    {'r', 255, 0, 7, 118}, // 'er'
    {'s', 255, 0, 7, 50}, // 'es'
    {'l', 255, 0, 8, 242}, // 'al'
    {'n', 57, 1, 7, 105}, // 'an'
    {'r', 255, 0, 8, 241}, // 'ar'
    {'s', 255, 0, 8, 177}, // 'as'
    {'t', 56, 1, 7, 70}, // 'at'
    {'e', 255, 0, 8, 174}, // 've'
    {',', 90, 1, 11, 435}, // ','
    {'-', 255, 0, 7, 30}, // '-'
    {'.', 23, 2, 8, 55}, // '.'
    {'e', 255, 0, 8, 187}, // 'se'
    {'t', 255, 0, 8, 214}, // 'st'
    {'1', 255, 0, 7, 86}, // '1'
    {'c', 255, 0, 8, 107}, // 'ic'
    {'n', 50, 1, 6, 12}, // 'in'
    {'o', 49, 1, 8, 176}, // 'io'
    {'s', 255, 0, 8, 246}, // 'is'
    {'t', 255, 0, 8, 243}, // 'it'
    {' ', 255, 0, 7, 31}, // '. '
    {'c', 62, 1, 15, 6945}, // '.c'
    {'n', 255, 0, 8, 22}, // 'tion'
    {':', 60, 1, 14, 3473}, // ':'
    {';', 58, 1, 14, 3475}, // ';'
    {'e', 255, 0, 7, 26}, // 'te'
    {'h', 81, 1, 6, 42}, // 'th'
    {'i', 64, 1, 7, 49}, // 'ti'
    {'o', 255, 0, 8, 215}, // 'to'
    {'a', 255, 0, 8, 105}, // 'ra'
    {'e', 255, 0, 7, 94}, // 're'
    {'i', 255, 0, 8, 133}, // 'ri'
    {'o', 255, 0, 8, 108}, // 'ro'
    {'f', 255, 0, 8, 247}, // 'of'
    {'n', 255, 0, 7, 92}, // 'on'
    {'r', 255, 0, 7, 42}, // 'or'
    {'u', 255, 0, 8, 186}, // 'ou'
    {'d', 255, 0, 7, 51}, // 'nd'
    {'e', 255, 0, 8, 97}, // 'ne'
    {'g', 255, 0, 8, 208}, // 'ng'
    {'t', 255, 0, 8, 240}, // 'nt'
    {'a', 255, 0, 8, 190}, // 'ha'
    {'e', 255, 0, 6, 34}, // 'he'
    {'i', 255, 0, 8, 135}, // 'hi'
    {'t', 52, 1, 10, 216}, // 'ht'
    {'e', 255, 0, 8, 175}, // 'le'
    {'n', 255, 0, 8, 109}, // 'ion'
    {'g', 255, 0, 8, 142}, // 'ing'
    {'p', 255, 0, 9, 109}, // 'http'
    {'t', 51, 1, 15, 6944}, // 'htt'
    {'t', 255, 0, 9, 419}, // 'ent'
    {'e', 255, 0, 8, 134}, // 'de'
    {'o', 255, 0, 9, 264}, // 'atio'
    {'i', 55, 1, 9, 265}, // 'ati'
    {'d', 255, 0, 8, 191}, // 'and'
    {'\x0a', 255, 0, 8, 57}, // ';\n'
    {'/', 255, 0, 8, 56}, // '://'
    {'/', 59, 1, 14, 3474}, // ':/'
    {'m', 255, 0, 9, 418}, // '.com'
    {'o', 61, 1, 12, 869}, // '.co'
    {'_', 255, 0, 5, 19}, // '_'
    {'o', 25, 1, 8, 23}, // 'tio'
    {'a', 6, 5, 4, 4}, // 'a'
    {'b', 255, 0, 7, 89}, // 'b'
    {'c', 74, 2, 6, 58}, // 'c'
    {'d', 54, 1, 5, 3}, // 'd'
    {'e', 1, 5, 4, 12}, // 'e'
    {'f', 255, 0, 6, 32}, // 'f'
    {'g', 255, 0, 6, 4}, // 'g'
    {'h', 44, 4, 5, 20}, // 'h'
    {'i', 18, 5, 4, 0}, // 'i'
    {'e', 255, 0, 8, 96}, // 'ce'
    {'o', 255, 0, 8, 106}, // 'co'
    {'l', 48, 1, 5, 11}, // 'l'
    {'m', 88, 1, 6, 36}, // 'm'
    {'n', 40, 4, 5, 31}, // 'n'
    {'o', 36, 4, 4, 2}, // 'o'
    {'p', 255, 0, 6, 20}, // 'p'
    {'e', 255, 0, 7, 122}, // 'the'
    {'r', 32, 4, 5, 27}, // 'r'
    {'s', 15, 2, 5, 28}, // 's'
    {'t', 28, 4, 4, 7}, // 't'
    {'u', 255, 0, 6, 45}, // 'u'
    {'v', 11, 1, 7, 43}, // 'v'
    {'w', 255, 0, 7, 106}, // 'w'
    {'e', 255, 0, 8, 143}, // 'me'
    {'y', 255, 0, 7, 119}, // 'y'
    {' ', 255, 0, 7, 29} // ', '
};

namespace test_data {
static constexpr std::string_view example = "tion"; // Example long string contained in the trie
static constexpr char missingShort = 'x'; // Example character not in the trie, but in range of min, max
static constexpr std::string_view missingLong = "ty"; // Example tring not in the trie, with only last character mismatch
}
}

namespace trie {

/// Return pointer to a node in trie corresponding to a first character c, or nullptr.
constexpr const Node* lookup_first_char(char c) {
    if (c < firstLevelMin || c > firstLevelMax)
        return nullptr; // Out of range

    auto ret = &flattened[c - firstLevelMin];

    if (ret->c != c)
        return nullptr; // In range, but not in trie
    else
        return ret;
}

/// Return pointer to a node in trie corresponding to a character c following
/// trie node base or nullptr.
constexpr const Node* lookup_next_char(const Node* base, char c) {
    if (base->childCount == 0)
        return nullptr; // Shortcut to prevent UB with pointer arithmetic outside array bounds

    const auto childrenStart = &flattened[base->childIndex];
    const auto childrenEnd = childrenStart + base->childCount;
    const auto ret = std::find_if(
        childrenStart, childrenEnd,
        [&](const auto& node) { return node.c == c; }
    );

    if (ret == childrenEnd)
        return nullptr;
    else
        return ret;
}

/// Read characters from the string and find longest available match in the trie.
/// Returns tuple of a pointer to the trie array and number of characters consumed.
constexpr std::pair<const Node*, size_t> lookup_symbol(std::string_view str) {
    auto current = lookup_first_char(str[0]);
    if (!current)
        return {nullptr, 0u};

    for (size_t i = 1; i < str.size(); ++i) {
        const auto next = lookup_next_char(current, str[i]);

        if (!next)
            return {current, i}; // Can't match any more characters

        current = next;
    }

    return {current, str.size()}; // Ran out of characters
}

}

template <typename BW>
size_t encode_nonmatch_group(std::string_view str, BW& bitWriter) {
    const auto groupEndIt = std::find_if(
        str.begin(), str.end(),
        [&](char c) { return trie::lookup_first_char(c) != nullptr; }
    );

    const size_t count = groupEndIt - str.begin();
    ::tablog::detail::elias_gamma(count, bitWriter);

    // Plain copy of the characters
    std::for_each(str.begin(), groupEndIt, [&](char c) { bitWriter.write(static_cast<uint8_t>(c)); });

    return count;
}

template <typename BW>
size_t encode_match_group(std::string_view str, BW& bitWriter) {
    size_t count = 0;
    size_t pos = 0;

    while (true) {
        auto [node, charsUsed] = trie::lookup_symbol(str.substr(pos));
        if (!node) {
            assert(charsUsed == 0);
            break;
        }

        assert(charsUsed > 0);
        assert(pos + charsUsed <= str.size());
        count += 1;
        pos += charsUsed;
    }

    ::tablog::detail::elias_gamma(count, bitWriter);

    pos = 0;
    for (size_t i = 0; i < count; ++i) {
        auto [node, charsUsed] = trie::lookup_symbol(str.substr(pos));
        assert(node);

        bitWriter.write(node->encodedValue, node->encodedLength);
        pos += charsUsed;
    }

    return count;
}

}

/// Compress a string into a bit writer, using a very small fixed dictionary.
/// This is designed to work well with short (preferably lower case) english text,
/// or (to a lesser extent URLs or code.
/// Works very poorly on binary data, unicode text is borderline,
/// depending on the ratio of non-latin characters (eg. should work ok on czech
/// with just a couple accented characters, very poorly on chinese).
template <typename BW>
void compress_string(std::string_view str, BW& bitWriter) {
    // Compressed format is a sequence of alternating groups of matching and non-matching
    // characters, each prefixed with length encoded using Ellias gamma encoding.
    //
    // Matching group length is the number of (variable length) symbols matched from
    // the trie (-> character length will typically be longer), content of a matching
    // group are the huffman codes of the matched symbols.
    // Non matching group length is the number of non matched characters followed
    // by the characters themselves.
    //
    // For every group except  the first one zero length means end of string,
    // for first group zero length means that the string begins with non-matching
    // data.
    // Empty string is encoded as two zero length groups.

    using namespace ::tablog::string::detail;

    size_t charactersUsed;

    // We unroll the first half iteration of the loop, because of how the empty
    // first match block handling is done (missing break between the two encodes).

    charactersUsed = encode_match_group(str, bitWriter);
    str = str.substr(charactersUsed);

    while (str.size()) {
        charactersUsed = encode_nonmatch_group(str, bitWriter);
        str = str.substr(charactersUsed);
        assert(charactersUsed);

        if (!str.size())
            break;

        charactersUsed = encode_match_group(str, bitWriter);
        str = str.substr(charactersUsed);
        assert(charactersUsed);
    }

    // The terminating zero
    ::tablog::detail::elias_gamma(0u, bitWriter);
}

}
