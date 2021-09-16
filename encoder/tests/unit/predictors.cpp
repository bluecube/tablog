#include <catch2/catch.hpp>

#include "predictors.hpp"

TEST_CASE("SimpleLinear predictor") {
    constexpr std::size_t n = 5;
    auto test_data = [](int i) { return 12 - 7 * i; };

    tablog::predictors::SimpleLinear<int, n> predictor;

    REQUIRE(predictor.predict() == 0);
        // Return zero if no data have been provided so far

    SECTION("Predictor is actually linear") {
        // skip over the initial zeros
        for (std::size_t i = 0; i < n; ++i)
            predictor.feed(test_data(i));

        for (std::size_t i = n; i < 10 * n; ++i)
        {
            predictor.feed(test_data(i));
            REQUIRE(predictor.predict() == test_data(i + 1));
        }
    }

    SECTION("Ignoring noise in the middle") {
        predictor.feed(test_data(0));
        for (unsigned i = 1; i < n - 1; ++i)
            predictor.feed(999);
        predictor.feed(test_data(n - 1));
        REQUIRE(predictor.predict() == test_data(n));
    }
}
