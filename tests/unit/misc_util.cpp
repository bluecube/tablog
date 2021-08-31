#include <catch2/catch.hpp>

#include "util/misc.hpp"

TEST_CASE("abs_diff uint8_t") {
    auto params = GENERATE(Catch::Generators::values({
        std::make_pair(std::make_pair(0u, 0u), std::make_pair(0u, false)),
        std::make_pair(std::make_pair(10u, 5u), std::make_pair(5u, true)),
        std::make_pair(std::make_pair(5u, 10u), std::make_pair(5u, false)),
        std::make_pair(std::make_pair(255u, 0u), std::make_pair(255u, true)),
        std::make_pair(std::make_pair(0u, 255u), std::make_pair(255u, false))
    }));
    const auto [inputs, expected] = params;
    auto ret = tablog::detail::abs_diff<uint8_t>(inputs.first, inputs.second);

    static_assert(std::is_same_v<decltype(ret.first), uint8_t>, "Returning unsigned type");

    REQUIRE(ret.first == expected.first);
    REQUIRE(ret.second == expected.second);
}

TEST_CASE("abs_diff int8_t") {
    auto params = GENERATE(Catch::Generators::values({
        std::make_pair(std::make_pair(0, 0), std::make_pair(0u, false)),
        std::make_pair(std::make_pair(10, -5), std::make_pair(15u, true)),
        std::make_pair(std::make_pair(-5, 10), std::make_pair(15u, false)),
        std::make_pair(std::make_pair(127, -128), std::make_pair(255u, true)),
        std::make_pair(std::make_pair(-128, 127), std::make_pair(255u, false)),
    }));
    const auto [inputs, expected] = params;
    auto ret = tablog::detail::abs_diff<int8_t>(inputs.first, inputs.second);

    static_assert(std::is_same_v<decltype(ret.first), uint8_t>, "Returning unsigned type");

    REQUIRE(ret.first == expected.first);
    REQUIRE(ret.second == expected.second);
}

TEST_CASE("abs_diff double") {
    auto params = GENERATE(Catch::Generators::values({
        std::make_pair(std::make_pair(0.0, 0.0), std::make_pair(0.0, false)),
        std::make_pair(std::make_pair(10.0, -5.0), std::make_pair(15.0, true)),
        std::make_pair(std::make_pair(-5.0, 10.0), std::make_pair(15.0, false)),
        std::make_pair(std::make_pair(127.0, -128.0), std::make_pair(255.0, true)),
        std::make_pair(std::make_pair(-128.0, 127.0), std::make_pair(255.0, false)),
        std::make_pair(std::make_pair(1.0e30, 1.001e30), std::make_pair(1.0e27, false))
    }));
    const auto [inputs, expected] = params;
    auto ret = tablog::detail::abs_diff<double>(inputs.first, inputs.second);

    static_assert(std::is_same_v<decltype(ret.first), double>, "Returning double");

    REQUIRE(ret.first == Approx(expected.first));
    REQUIRE(ret.second == expected.second);
}
