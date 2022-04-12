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

// The following code was generated from the jupyter notebook string_predictors.ipynb
static constexpr char firstLevelMin = ' ';
static constexpr char firstLevelMax = 'y';
static constexpr Node flattened[] = {
    {' ', 255, 0, 6, 41}, // ' '
    {'a', 255, 0, 8, 22}, // 'ea'
    {'d', 255, 0, 7, 40}, // 'ed'
    {'n', 53, 1, 7, 118}, // 'en'
    {'r', 255, 0, 7, 55}, // 'er'
    {'s', 255, 0, 7, 38}, // 'es'
    {'l', 255, 0, 8, 79}, // 'al'
    {'n', 57, 1, 7, 75}, // 'an'
    {'r', 255, 0, 8, 143}, // 'ar'
    {'s', 255, 0, 8, 141}, // 'as'
    {'t', 56, 1, 7, 49}, // 'at'
    {'e', 255, 0, 8, 117}, // 've'
    {',', 90, 1, 11, 1644}, // ','
    {'-', 255, 0, 7, 60}, // '-'
    {'.', 23, 2, 8, 236}, // '.'
    {'e', 255, 0, 8, 221}, // 'se'
    {'t', 255, 0, 8, 107}, // 'st'
    {'1', 255, 0, 7, 53}, // '1'
    {'c', 255, 0, 8, 214}, // 'ic'
    {'n', 50, 1, 6, 12}, // 'in'
    {'o', 49, 1, 8, 13}, // 'io'
    {'s', 255, 0, 8, 111}, // 'is'
    {'t', 255, 0, 8, 207}, // 'it'
    {' ', 255, 0, 7, 124}, // '. '
    {'c', 62, 1, 15, 17004}, // '.c'
    {'n', 255, 0, 8, 104}, // 'tion'
    {':', 60, 1, 14, 8812}, // ':'
    {';', 58, 1, 14, 12908}, // ';'
    {'e', 255, 0, 7, 44}, // 'te'
    {'h', 81, 1, 6, 21}, // 'th'
    {'i', 64, 1, 7, 70}, // 'ti'
    {'o', 255, 0, 8, 235}, // 'to'
    {'a', 255, 0, 8, 150}, // 'ra'
    {'e', 255, 0, 7, 61}, // 're'
    {'i', 255, 0, 8, 161}, // 'ri'
    {'o', 255, 0, 8, 54}, // 'ro'
    {'f', 255, 0, 8, 239}, // 'of'
    {'n', 255, 0, 7, 29}, // 'on'
    {'r', 255, 0, 7, 42}, // 'or'
    {'u', 255, 0, 8, 93}, // 'ou'
    {'d', 255, 0, 7, 102}, // 'nd'
    {'e', 255, 0, 8, 134}, // 'ne'
    {'g', 255, 0, 8, 11}, // 'ng'
    {'t', 255, 0, 8, 15}, // 'nt'
    {'a', 255, 0, 8, 125}, // 'ha'
    {'e', 255, 0, 6, 17}, // 'he'
    {'i', 255, 0, 8, 225}, // 'hi'
    {'t', 52, 1, 10, 108}, // 'ht'
    {'e', 255, 0, 8, 245}, // 'le'
    {'n', 255, 0, 8, 182}, // 'ion'
    {'g', 255, 0, 8, 113}, // 'ing'
    {'p', 255, 0, 9, 364}, // 'http'
    {'t', 51, 1, 15, 620}, // 'htt'
    {'t', 255, 0, 9, 395}, // 'ent'
    {'e', 255, 0, 8, 97}, // 'de'
    {'o', 255, 0, 9, 33}, // 'atio'
    {'i', 55, 1, 9, 289}, // 'ati'
    {'d', 255, 0, 8, 253}, // 'and'
    {'\x0a', 255, 0, 8, 156}, // ';\n'
    {'/', 255, 0, 8, 28}, // '://'
    {'/', 59, 1, 14, 4716}, // ':/'
    {'m', 255, 0, 9, 139}, // '.com'
    {'o', 61, 1, 12, 2668}, // '.co'
    {'_', 255, 0, 5, 25}, // '_'
    {'o', 25, 1, 8, 232}, // 'tio'
    {'a', 6, 5, 4, 2}, // 'a'
    {'b', 255, 0, 7, 77}, // 'b'
    {'c', 74, 2, 6, 23}, // 'c'
    {'d', 54, 1, 5, 24}, // 'd'
    {'e', 1, 5, 4, 3}, // 'e'
    {'f', 255, 0, 6, 1}, // 'f'
    {'g', 255, 0, 6, 8}, // 'g'
    {'h', 44, 4, 5, 5}, // 'h'
    {'i', 18, 5, 4, 0}, // 'i'
    {'e', 255, 0, 8, 6}, // 'ce'
    {'o', 255, 0, 8, 86}, // 'co'
    {'l', 48, 1, 5, 26}, // 'l'
    {'m', 88, 1, 6, 9}, // 'm'
    {'n', 40, 4, 5, 31}, // 'n'
    {'o', 36, 4, 4, 4}, // 'o'
    {'p', 255, 0, 6, 10}, // 'p'
    {'e', 255, 0, 7, 47}, // 'the'
    {'r', 32, 4, 5, 27}, // 'r'
    {'s', 15, 2, 5, 7}, // 's'
    {'t', 28, 4, 4, 14}, // 't'
    {'u', 255, 0, 6, 45}, // 'u'
    {'v', 11, 1, 7, 106}, // 'v'
    {'w', 255, 0, 7, 43}, // 'w'
    {'e', 255, 0, 8, 241}, // 'me'
    {'y', 255, 0, 7, 119}, // 'y'
    {' ', 255, 0, 7, 92} // ', '
};

namespace test_data {
static constexpr std::string_view example = "tion"; // Example long string contained in the trie
static constexpr char missingShort = 'x'; // Example character not in the trie, but in range of min, max
static constexpr std::string_view missingLong = "ty"; // Example tring not in the trie, with only last character mismatch
}

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
    if (str.empty())
        return {nullptr, 0u};

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

    return pos;
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
