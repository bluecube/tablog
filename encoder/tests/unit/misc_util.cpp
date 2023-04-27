#include "util/misc.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/catch_approx.hpp>

#include <limits>
#include <cmath>

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

    REQUIRE(ret.first == Catch::Approx(expected.first));
    REQUIRE(ret.second == expected.second);
}

TEMPLATE_TEST_CASE("extrapolate", "",
    uint8_t, int8_t,
    uint16_t, int16_t,
    uint32_t, int32_t,
    uint64_t, int64_t) {
    using T = TestType;

    static constexpr auto max = std::numeric_limits<T>::max();
    static constexpr auto min = std::numeric_limits<T>::min();

    auto params = GENERATE(
        std::make_pair(std::make_pair(0ll, 0ll), 0ll),
        std::make_pair(std::make_pair(10ll, 20ll), 25ll),
        std::make_pair(std::make_pair(20ll, 10ll), 5ll),
        std::make_pair(std::make_pair(max - 10ll, max), max),
        std::make_pair(std::make_pair(max, max - 10ll), max - 15ll),
        std::make_pair(std::make_pair(min + 10ll, min), min),
        std::make_pair(std::make_pair(min, min + 10ll), min + 15ll),
        std::make_pair(std::make_pair(min, max), max),
        std::make_pair(std::make_pair(max, min), min),
        std::make_pair(std::make_pair(1, 2), 2),
        std::make_pair(std::make_pair(2, 1), 1),
        std::make_pair(std::make_pair(1, 4), 5),
        std::make_pair(std::make_pair(7, 4), 3),
        std::make_pair(std::make_pair(max - 4ll, max - 2ll), max - 1ll),
        std::make_pair(std::make_pair(min + 4ll, min + 2ll), min + 1ll)
    );

    const auto [inputs, expected] = params;

    auto ret = tablog::detail::extrapolate<T, 2>(inputs.first, inputs.second);
    //std::cout << inputs.first << " " << inputs.second << " -> " << ret << " =?= " << expected << "\n";

    static_assert(std::is_same_v<decltype(ret), T>, "Returning same type");

    REQUIRE(static_cast<int64_t>(ret) == expected);
}

TEST_CASE("small_int_log2") {
    auto v = GENERATE(Catch::Generators::range(1, 9));
    REQUIRE(tablog::detail::small_int_log2(v) == floor(log2(v)));
}

TEMPLATE_TEST_CASE("int_log2", "",
    uint8_t, uint16_t, uint32_t, uint64_t) {

    auto v = GENERATE(
        Catch::Generators::take(
            500,
            Catch::Generators::random<uint64_t>(
                1ul,
                std::numeric_limits<TestType>::max()
            )
        )
    );
    REQUIRE(tablog::detail::int_log2(static_cast<TestType>(v)) == floor(log2(v)));
}
