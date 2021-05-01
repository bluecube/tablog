#include <catch2/catch.hpp>

#include "circular_buffer.hpp"

TEST_CASE("CircularBuffer") {
    constexpr int n = 5;
    tablog::util::CircularBuffer<int, n> c;

    REQUIRE(c.size() == 0);
    REQUIRE(c.capacity() == n);

    SECTION("with elements") {
        for (int i = 0; i < n; ++i)
        {
            c.push_back(i);
            REQUIRE(c.size() == i + 1);
        }

        SECTION("element_access") {
            for (int i = 0; i < n; ++i)
            {
                REQUIRE(c[i] == i);
            }
        }

        SECTION("pop") {
            for (int i = 0; i < n; ++i)
            {
                REQUIRE(c.pop_front() == i);
                REQUIRE(c.size() == n - i - 1);
            }
        }

        SECTION("overflow") {
            int max = 2 * n + n / 2;
            for (int i = 0; i < max; ++i)
            {
                c.push_back(i);
                REQUIRE(c.size() == n);
            }

            for (int i = 0; i < n; ++i)
            {
                REQUIRE(c[i] == max - n + i);
            }
        }

        SECTION("combined") {
            int max = 100;
            for (int i = 0; i < max; ++i)
            {
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
}
