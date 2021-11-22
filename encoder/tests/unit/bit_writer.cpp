#include "util/bit_writer.hpp"

#include <catch2/catch.hpp>

TEST_CASE("BitWriter") {
    std::string data;

    tablog::util::BitWriter bw([&data](uint8_t c) { data.push_back(c); });

    SECTION("Write 8 single bits") {
        for (uint32_t i = 0; i < 8; ++i)
            bw.write(i, 1);
        REQUIRE(data == "\xaa");
    }

    SECTION("Write 8 bits") {
        bw.write(static_cast<uint8_t>('A'), 8);
        REQUIRE(data == "A");
    }

    SECTION("Write 4x4 bits") {
        bw.write(0x6u, 4);
        bw.write(0x7u, 4);
        bw.write(0x8u, 4);
        bw.write(0x9u, 4);
        REQUIRE(data == "\x76\x98");
    }

    SECTION("Write 32bit number -- checks endiannes") {
        bw.write(327348935ull, 32);
        REQUIRE(data == "\xc7\xf2\x82\x13");
    }

    SECTION("Write 2x16bit number -- checks endiannes") {
        /// The same bytes as for 32bit write above, just split up
        bw.write(62151u, 16);
        bw.write(4994u, 16);
        REQUIRE(data == "\xc7\xf2\x82\x13");
    }

    SECTION("Flush 4 bits") {
        bw.write(0x9u, 4);
        bw.flush();
        REQUIRE(data == "\x09");
    }

    SECTION("Small write followed by large write followed by flush") {
        bw.write(0u, 1);
        bw.write(2474832583ull, 32);
        bw.flush();
        REQUIRE(data == "\x8e\xe5\x05\x27\x01");
    }

    SECTION("Upper bit masking") {
        bw.write(0xffu, 1);
        bw.flush();
    }

    SECTION("Write 8 bits bit by bit") {
        uint8_t c = 'A';
        for (unsigned i = 0; i < 8; ++i) {
            bw.write_bit(c);
            c >>= 1;
        }
        REQUIRE(data == "A");
    }

    SECTION("Byte pattern") {
        for (uint32_t i = 'a'; i < 'h'; ++i)
            bw.write(i, 8);

        REQUIRE(data == "abcdefg");
    }
}
