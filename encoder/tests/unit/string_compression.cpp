#include "string_compression.hpp"
#include "util/string_bit_writer.hpp"

#include <limits>
#include <iostream>
#include <random>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

using namespace tablog::string;
using namespace tablog::string::detail;

// Some assumptions to make the tests doable
constexpr int minChar = std::numeric_limits<char>::min();
constexpr int maxChar = std::numeric_limits<char>::max();
static_assert(trie::firstLevelMin > minChar);
static_assert(trie::firstLevelMax < maxChar);

auto constexpr nodeCount = sizeof(trie::flattened) / sizeof(*trie::flattened);

/// Generate a random string that is contained in te trie and that can't be
/// extended any more
class TrieStringGenerator: public Catch::Generators::IGenerator<std::pair<std::string, const trie::Node*>> {
public:
    TrieStringGenerator()
    : rand(std::random_device{}()) {
        next();
    }

    const std::pair<std::string, const trie::Node*>& get() const override {
        return value;
    }

    bool next() override {
        value.first.clear();
        auto current = gen_first_char();
        value.first.push_back(current->c);

        while (current->childCount > 0) {
            std::uniform_int_distribution<char> dist(0, current->childCount - 1);
            current = &trie::flattened[dist(rand) + current->childIndex];
            value.first.push_back(current->c);
        }

        value.second = current;

        return true;
    }

protected:
    const trie::Node* gen_first_char() {
        static std::uniform_int_distribution<char> firstLevelDist(trie::firstLevelMin, trie::firstLevelMax);
        while (true) {
            char c = firstLevelDist(rand);
            const auto ret = trie::lookup_first_char(c);
            if (ret)
                return ret;
        }

        return nullptr;
    }

    std::default_random_engine rand;
    std::uniform_int_distribution<> dist;
    std::pair<std::string, const trie::Node*> value;
};

/// Test that first level items don't have any incoming links in the trie.
TEST_CASE("String compression: Flattened trie incoming edges") {
    for (unsigned i = 0; i < nodeCount; ++i) {
        const bool isCorrespondingValue = (trie::flattened[i].c == static_cast<char>(trie::firstLevelMin + i));

        if (!isCorrespondingValue)
            continue; // Nodes on other than first level might or might not have incoming links

        for (unsigned j = 0; j < nodeCount; ++j) {
            const auto childrenStartIndex = trie::flattened[j].childIndex;
            const auto childrenEndIndex = trie::flattened[j].childCount;

            bool isIncomingLink = (i >= childrenStartIndex && i < childrenEndIndex);
            REQUIRE(!isIncomingLink);
        }
    }
}

TEST_CASE("String compression: First character false") {
    const auto c = GENERATE(
        trie::firstLevelMin - 1,
        trie::firstLevelMax + 1,
        take(
            10,
            random(minChar, trie::firstLevelMin - 1)
        ),
        take(
            10,
            random(trie::firstLevelMax + 1, maxChar)
        ),
        trie::test_data::missingShort
    );

    REQUIRE(detail::trie::lookup_first_char(c) == nullptr);
}

TEST_CASE("String compression: First character true") {
    const auto c = GENERATE(
        static_cast<int>(trie::firstLevelMin),
        static_cast<int>(trie::firstLevelMax),
        static_cast<int>(trie::test_data::example[0])
    );

    const auto ret = trie::lookup_first_char(c);
    REQUIRE(ret != nullptr);
    REQUIRE(ret->c == c);
}

TEST_CASE("String compression: Next character true") {
    const auto i = GENERATE(
        Catch::Generators::take(
            20,
            Catch::Generators::filter(
                [](auto i) { return trie::flattened[i].childCount > 0; },
                Catch::Generators::random<unsigned>(0, nodeCount - 1)
            )
        )
    );
    const auto& node = trie::flattened[i];

    const auto nextCharFloat = GENERATE(
        Catch::Generators::take(
            3,
            Catch::Generators::random(0.0, 1.0)
        )
    );
    const auto nextChar = trie::flattened[node.childIndex + size_t(nextCharFloat * node.childCount)].c;

    const auto nextNode = trie::lookup_next_char(&node, nextChar);
    REQUIRE(nextNode != nullptr);
    REQUIRE(nextNode->c == nextChar);
}

TEST_CASE("String compression: Next character false") {
    const auto i = GENERATE(
        Catch::Generators::take(
            20,
            Catch::Generators::random<unsigned>(0, nodeCount - 1)
        )
    );
    const auto& node = trie::flattened[i];

    // Working around inability of Catch2 to make generator dependent on other values.
    // Construct a vector of characters that are not successors of node, then select
    // one of them using a floating point index.
    std::vector<char> nextCharsNotInNode;
    for (uint j = 0; j < 255; ++j) {
        const char c = static_cast<char>(j);

        bool cInNext = [&]() {
            for (uint i = 0; i < node.childCount; ++i)
                if (trie::flattened[i + node.childIndex].c == c)
                    return true;
            return false;
        }();

        if (!cInNext)
            nextCharsNotInNode.push_back(c);
    };
    const auto nextCharFloat = GENERATE(
        Catch::Generators::take(
            3,
            Catch::Generators::random(0.0, 1.0)
        )
    );
    const auto nextChar = nextCharsNotInNode[size_t(nextCharFloat * nextCharsNotInNode.size())];

    const auto nextNode = trie::lookup_next_char(&node, nextChar);
    REQUIRE(nextNode == nullptr);
}

TEST_CASE("String compression: Lookup symbol bad first char") {
    const auto c = GENERATE(
        trie::firstLevelMin - 1,
        trie::firstLevelMax + 1,
        take(
            10,
            random(minChar, trie::firstLevelMin - 1)
        ),
        take(
            10,
            random(trie::firstLevelMax + 1, maxChar)
        ),
        take(
            10,
            filter(
                [](int c) { return trie::lookup_first_char(c) == nullptr; },
                random<int>(detail::trie::firstLevelMin, detail::trie::firstLevelMax)
            )
        ),
        trie::test_data::missingShort
    );

    char c2 = c;
    std::string_view sv(&c2, 1);

    const auto result = trie::lookup_symbol(sv);
    REQUIRE(result.first == nullptr);
    REQUIRE(result.second == 0);
}

TEST_CASE("String compression: Lookup symbol match") {
    auto [matchingString, node] = GENERATE(
        take(
            50,
            Catch::Generators::GeneratorWrapper<std::pair<std::string, const trie::Node*>>(
                new TrieStringGenerator()
            )
        )
    );

    std::string input;

    SECTION("With exact length input") {
        input = matchingString;
    }

    SECTION("Followed by more data") {
        char following = GENERATE(
            values<int>({
                trie::test_data::example[0],
                trie::test_data::missingShort
            }),
            take(
                20,
                Catch::Generators::random<int>('\0', '\xff')
            )
        );

        input = matchingString + following;
    }

    const auto result = trie::lookup_symbol(input);
    REQUIRE(result.first == node);
    REQUIRE(result.first->c == matchingString[matchingString.size() - 1]);
    REQUIRE(result.second == matchingString.size());
}

TEST_CASE("String compression: Lookup symbol empty") {
    const auto result = trie::lookup_symbol(std::string_view());
    REQUIRE(result.first == nullptr);
    REQUIRE(result.second == 0);
}

/// Check that compression doesn't fail on pre-selected inputs.
/// This is mostly  using only the internal asserts as without a decoder we can't
/// check correctness too well.
TEST_CASE("String compression smoke tests") {
    tablog::util::StringBitWriter bwStr;

    /// Check that empty string consists of exactly two 1 bits.
    /// This is mostly an implementation detail, but it verifies that the 
    /// encoding is not completely off.
    SECTION("Empty string") {
        compress_string("", bwStr);
        bwStr.flush_last_byte();

        tablog::util::StringBitWriter bwExpected;
        bwExpected.write(0b11u, 2);
        bwExpected.flush_last_byte();

        REQUIRE(bwStr.data == bwExpected.data);
    }

    SECTION("Long example") {
        compress_string(trie::test_data::example, bwStr);
    }

    SECTION("Non-alpha") {
        compress_string("@#$%^", bwStr);
    }

    SECTION("Missing long") {
        compress_string(trie::test_data::missingLong, bwStr);
    }
}
