#include "util/string_bit_writer.hpp"

#include <catch2/catch.hpp>

TEST_CASE("BitWriter") {
    tablog::util::StringBitWriter bw;

    SECTION("Write 8 single bits") {
        for (uint32_t i = 0; i < 8; ++i)
            bw.write(i, 1);
        REQUIRE(bw.data == "\xaa");
    }

    SECTION("Write 8 bits") {
        bw.write(static_cast<uint8_t>('A'), 8);
        REQUIRE(bw.data == "A");
    }

    SECTION("Write 4x4 bits") {
        bw.write(0x6u, 4);
        bw.write(0x7u, 4);
        bw.write(0x8u, 4);
        bw.write(0x9u, 4);
        REQUIRE(bw.data == "\x76\x98");
    }

    SECTION("Write 32bit number -- checks endiannes") {
        bw.write(327348935ull, 32);
        REQUIRE(bw.data == "\xc7\xf2\x82\x13");
    }

    SECTION("Write 2x16bit number -- checks endiannes") {
        /// The same bytes as for 32bit write above, just split up
        bw.write(62151u, 16);
        bw.write(4994u, 16);
        REQUIRE(bw.data == "\xc7\xf2\x82\x13");
    }

    SECTION("Flush 4 bits") {
        bw.write(0x9u, 4);
        bw.flush_last_byte();
        REQUIRE(bw.data == "\x09");

        SECTION("Double flush is no-op") {
            bw.flush_last_byte();
            REQUIRE(bw.data == "\x09");
        }
    }

    SECTION("End 4 bits") {
        bw.write(0x9u, 4);
        bw.end_bit_stream();
        REQUIRE(bw.data == "\x19");
    }

    SECTION("End, followed by the same data") {
        bw.write(0xdeadcafe, 27); // Just an arbitrary partial byte pattern
        bw.end_bit_stream();
        bw.write(0xdeadcafe, 27); // Just an arbitrary partial byte pattern
        bw.end_bit_stream();
        REQUIRE(bw.data.size() == 8);
        REQUIRE(bw.data.substr(0, 4) == bw.data.substr(4, 4));
    }

    SECTION("Small write followed by large write followed by flush") {
        bw.write(0u, 1);
        bw.write(2474832583ull, 32);
        bw.flush_last_byte();
        REQUIRE(bw.data == "\x8e\xe5\x05\x27\x01");
    }

    SECTION("Upper bit masking") {
        bw.write(0xffu, 1);
        bw.flush_last_byte();
        REQUIRE(bw.data == "\x01");
    }

    SECTION("Write 8 bits bit by bit") {
        uint8_t c = 'A';
        for (unsigned i = 0; i < 8; ++i) {
            bw.write_bit(c);
            c >>= 1;
        }
        REQUIRE(bw.data == "A");
    }

    SECTION("Byte pattern") {
        for (uint32_t i = 'a'; i < 'h'; ++i)
            bw.write(i, 8);

        REQUIRE(bw.data == "abcdefg");
    }

    SECTION("Bit pattern") {
        auto generateExpected = [&](uint32_t patternLength) {
            const auto fullBytes = patternLength / 8u;
            std::string ret(fullBytes, '\xaa');

            assert(ret.size() == fullBytes);

            const auto remainingBits = patternLength - 8u * fullBytes;
            if (remainingBits)
                ret.push_back(0xaau & ((1u << remainingBits) - 1u));

            assert(ret.size() == (patternLength + 7u) / 8u);

            return ret;
        };

        SECTION("Single bits") {
            const auto patternLength = GENERATE(Catch::Generators::take(
                100,
                Catch::Generators::random(1u, 2048u)
            ));

            const auto expected = generateExpected(patternLength);

            for (uint32_t i = 0; i < patternLength; ++i)
                bw.write_bit(i);
            bw.flush_last_byte();

            REQUIRE(bw.data == expected);
        }

        SECTION("Chunks") {
            auto writeLength = GENERATE(Catch::Generators::range(1u, 8u));
            auto writeCount = GENERATE(Catch::Generators::take(
                20,
                Catch::Generators::random(1u, 100u)
            ));

            auto writeData = 0xaau & ((1 << writeLength) - 1);

            const auto expected = generateExpected(writeCount * writeLength);


            for (uint32_t i = 0; i < writeCount; ++i) {
                bw.write(writeData, writeLength);
                if (writeLength % 2)
                    writeData = ~writeData; // Invert the writeData if it's odd length
            }
            bw.flush_last_byte();

            REQUIRE(bw.data == expected);
        }
    }

    SECTION("Empty write") {
        bw.write(0x6u, 4);
        bw.write(0xffu, 0);
        bw.write(0x7u, 4);
        REQUIRE(bw.data == "\x76");
    }
}
