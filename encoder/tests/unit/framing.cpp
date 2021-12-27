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
        f.start();
        REQUIRE(data == "Tl");
    }

    SECTION("Only end") {
        f.end();
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
}
