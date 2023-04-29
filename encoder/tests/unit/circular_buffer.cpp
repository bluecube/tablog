#include <catch2/catch_test_macros.hpp>

#include "util/circular_buffer.hpp"

TEST_CASE("CircularBuffer") {
    constexpr unsigned n = 5;
    tablog::util::CircularBuffer<unsigned, n> c;

    REQUIRE(c.size() == 0);
    REQUIRE(c.empty());
    REQUIRE(!c.full());
    REQUIRE(c.capacity() == n);

    SECTION("full of elements") {
        for (unsigned i = 0; i < n; ++i) {
            c.push_back(i);
            REQUIRE(c.size() == i + 1);
        }
        REQUIRE(!c.empty());
        REQUIRE(c.full());

        SECTION("element_access") {
            for (unsigned i = 0; i < n; ++i)
                REQUIRE(c[i] == i);
            REQUIRE(c.front() == 0);
            REQUIRE(c.back() == n - 1);
        }

        SECTION("pop") {
            for (unsigned i = 0; i < n; ++i) {
                REQUIRE(c.pop_front() == i);
                REQUIRE(c.size() == n - i - 1);
                REQUIRE(!c.full());
            }
            REQUIRE(c.empty());
        }

        SECTION("overflow") {
            unsigned max = 2 * n + n / 2;
            for (unsigned i = 0; i < max; ++i) {
                c.push_back(i);
                REQUIRE(c.size() == n);
            }

            for (unsigned i = 0; i < n; ++i)
                REQUIRE(c[i] == max - n + i);
        }

        SECTION("combined") {
            unsigned max = 11;
            for (unsigned i = 0; i < max; ++i) {
                c.pop_front();
                c.pop_front();
                REQUIRE(c.size() == n - 2);
                c.push_back(i);
                c.push_back(2 * i);
                c.push_back(3 * i);
                REQUIRE(c.size() == n);
            }
        }

        SECTION("clear") {
            c.clear();
            REQUIRE(c.empty());
        }
    }

    SECTION("single element front and back are the same") {
        c.push_back(42);
        REQUIRE(c.front() == 42);
        REQUIRE(c.back() == 42);
    }

    SECTION("chasing single element") {
        c.push_back(0);
        for (unsigned i = 0; i < 10; ++i) {
            REQUIRE(c.size() == 1);
            REQUIRE(c.front() == i);
            REQUIRE(c.pop_front() == i);
            REQUIRE(c.empty());
            c.push_back(i + 1);
            REQUIRE(c.size() == 1);
        }
    }
}

TEST_CASE("CircularBufferFixed") {
    constexpr unsigned n = 5;
    constexpr unsigned defaultValue = 100;
    tablog::util::CircularBufferFixed<unsigned, n> c(defaultValue);

    SECTION("default values") {
        REQUIRE(c.size() == n);
        REQUIRE(!c.empty());
        REQUIRE(c.capacity() == n);

        for (unsigned i = 0; i < n; ++i)
            REQUIRE(c[i] == defaultValue);
    }

    SECTION("with elements") {
        for (unsigned i = 0; i < n; ++i)
            c.push_back(i);

        SECTION("element_access") {
            for (unsigned i = 0; i < n; ++i)
                REQUIRE(c[i] == i);
            REQUIRE(c.front() == 0);
            REQUIRE(c.back() == n - 1);
        }

        SECTION("overflow") {
            unsigned max = 2 * n + n / 2;
            for (unsigned i = 0; i < max; ++i)
                c.push_back(i);

            for (unsigned i = 0; i < n; ++i)
                REQUIRE(c[i] == max - n + i);
        }
    }
}
