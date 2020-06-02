#include "optional.hpp"

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

TEST_CASE("Emplace", "[emplace]")
{
    dze::optional<std::pair<std::pair<int, int>, std::pair<double, double>>> i;
    i.emplace(std::piecewise_construct, std::make_tuple(0, 1), std::make_tuple(2, 3));
    REQUIRE(i.has_value());
    CHECK(i->first.first == 0);
    CHECK(i->first.second == 1);
    CHECK(i->second.first == 2);
    CHECK(i->second.second == 3);
}
