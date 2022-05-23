#include "framing.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Framing") {
    std::string data;

    tablog::detail::Framing f([&data](uint8_t c) { data.push_back(c); });

    SECTION("Simple string") {
        const std::string s = "this is just a test";
        for (const auto c: s)
            f(c);

        REQUIRE(data == s);
    }

    SECTION("Only start") {
        f.start_of_block();
        REQUIRE(data == "Tl");
    }

    SECTION("Only end") {
        f.end_of_block();
        REQUIRE(data == "T#");
    }

    SECTION("Escape characters") {
        auto secondChar = GENERATE('l', '#', ' ');
        f('T');
        f(secondChar);
        REQUIRE(data == std::string("T ") + secondChar);
    }

    SECTION("Escape followed by boring") {
        f('T');
        f('x');
        REQUIRE(data == "Tx");
    }

    SECTION("Double escape followed by boring") {
        f('T');
        f('T');
        f('x');
        REQUIRE(data == "TTx");
    }

    SECTION("Double escape followed by important") {
        auto secondChar = GENERATE('l', '#', ' ');
        f('T');
        f('T');
        f(secondChar);
        REQUIRE(data == std::string("TT ") + secondChar);
    }

    SECTION("Compactness") {
        // Iterate over all pairs of characters, check that the string expands
        // by a defined amount
        for (uint32_t a = 0; a <= 255; ++a) {
            for (uint32_t b = 0; b <= 255; ++b) {
                f(a);
                f(b);
                f(',');
                    // Separate by a boring character, so that we never encounter
                    // a and b in reverse order
            }
        }

        REQUIRE(data.size() == 256 * 256 * 3 + 3);
        // We wrote 256 * 256 * 3 bytes,
        // three pairs of bytes are escaped: "Tl", "T#" and "T "
    }
}
