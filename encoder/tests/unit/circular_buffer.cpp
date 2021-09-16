#include <catch2/catch.hpp>

#include "util/circular_buffer.hpp"

TEST_CASE("CircularBuffer") {
    constexpr int n = 5;
    tablog::util::CircularBuffer<int, n> c;

    REQUIRE(c.size() == 0);
    REQUIRE(c.empty());
    REQUIRE(c.capacity() == n);

    SECTION("with elements") {
        for (int i = 0; i < n; ++i) {
            c.push_back(i);
            REQUIRE(c.size() == i + 1);
        }

        REQUIRE(!c.empty());

        SECTION("element_access") {
            for (int i = 0; i < n; ++i)
                REQUIRE(c[i] == i);
            REQUIRE(c.front() == 0);
            REQUIRE(c.back() == n - 1);
        }

        SECTION("pop") {
            for (int i = 0; i < n; ++i) {
                REQUIRE(c.pop_front() == i);
                REQUIRE(c.size() == n - i - 1);
            }
        }

        SECTION("overflow") {
            int max = 2 * n + n / 2;
            for (int i = 0; i < max; ++i) {
                c.push_back(i);
                REQUIRE(c.size() == n);
            }

            for (int i = 0; i < n; ++i)
                REQUIRE(c[i] == max - n + i);
        }

        SECTION("combined") {
            int max = 11;
            for (int i = 0; i < max; ++i) {
                c.pop_front();
                c.pop_front();
                REQUIRE(c.size() == n - 2);
                c.push_back(i);
                c.push_back(2 * i);
                c.push_back(3 * i);
                REQUIRE(c.size() == n);
            }
        }
    }

    SECTION("single element front and back are the same") {
        c.push_back(42);
        REQUIRE(c.front() == 42);
        REQUIRE(c.back() == 42);
    }
}

TEST_CASE("CircularBufferFixed") {
    constexpr int n = 5;
    constexpr int defaultValue = 100;
    tablog::util::CircularBufferFixed<int, n> c(defaultValue);

    SECTION("default values") {
        REQUIRE(c.size() == n);
        REQUIRE(!c.empty());
        REQUIRE(c.capacity() == n);

        for (int i = 0; i < n; ++i)
            REQUIRE(c[i] == defaultValue);
    }

    SECTION("with elements") {
        for (int i = 0; i < n; ++i)
            c.push_back(i);

        SECTION("element_access") {
            for (int i = 0; i < n; ++i)
                REQUIRE(c[i] == i);
            REQUIRE(c.front() == 0);
            REQUIRE(c.back() == n - 1);
        }

        SECTION("overflow") {
            int max = 2 * n + n / 2;
            for (int i = 0; i < max; ++i)
                c.push_back(i);

            for (int i = 0; i < n; ++i)
                REQUIRE(c[i] == max - n + i);
        }
    }
}
