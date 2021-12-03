#include <catch2/catch.hpp>

#include "stream_encoder_bits.hpp"
#include "util/bit_writer.hpp"

TEST_CASE("Adaptive exp-golomb encoding") {
    tablog::detail::AdaptiveExpGolombEncoder<uint64_t> encoder;

    std::string data;
    tablog::util::BitWriter bw([&data](uint8_t c) { data.push_back(c); });

    SECTION("Can encode value without throwing") {
        const auto values = GENERATE(Catch::Generators::take(
            100,
            Catch::Generators::chunk(
                1000,
                Catch::Generators::random(1u, 20000u)
            )
        ));

        for (auto v: values)
            encoder.encode(v, bw);
    }

    SECTION("Encoding zeroes makes state go to zero") {
        REQUIRE(encoder.get_state() != 0);

        for (uint32_t i = 0; i < 255; ++i)
            encoder.encode(0, bw);

        REQUIRE(uint32_t(encoder.get_state()) == 0);
    }

    SECTION("Encoding max values makes state go to max") {
        REQUIRE(encoder.get_state() != 0);

        for (uint32_t i = 0; i < 255; ++i)
            encoder.encode(std::numeric_limits<uint64_t>::max(), bw);

        // The exact value is not important here, just check that it's large
        // and not chaning any more
        const uint32_t tmpState = encoder.get_state();
        REQUIRE(tmpState > 128);

        for (uint32_t i = 0; i < 50; ++i)
            encoder.encode(std::numeric_limits<uint64_t>::max(), bw);

        REQUIRE(static_cast<uint32_t>(encoder.get_state()) == tmpState);
    }
}
