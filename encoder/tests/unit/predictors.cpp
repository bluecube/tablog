#include <catch2/catch.hpp>

#include "predictors.hpp"

TEST_CASE("SimpleLinear predictor") {
    constexpr std::size_t n = 5;
    auto test_data = [](int i) { return 12 - 7 * i; };

    tablog::predictors::SimpleLinear<int, n> predictor;

    REQUIRE(predictor.predict_and_feed(0) == 0);
        // Return zero if no data have been provided so far

    SECTION("Predictor is actually linear") {
        // skip over the initial zeros
        for (std::size_t i = 0; i < n; ++i)
            predictor.predict_and_feed(test_data(i));

        for (std::size_t i = n; i < 10 * n; ++i)
        {
            auto v = test_data(i);
            REQUIRE(predictor.predict_and_feed(v) == v);
        }
    }

    SECTION("Ignoring noise in the middle") {
        predictor.predict_and_feed(test_data(0));
        for (unsigned i = 1; i < n - 1; ++i)
            predictor.predict_and_feed(999);
        predictor.predict_and_feed(test_data(n - 1));
        REQUIRE(predictor.predict_and_feed(0) == test_data(n));
    }
}
