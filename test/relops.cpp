#include "optional.hpp"

#include <optional>
#include <string>

#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE(
    "Relational ops", "[relops]", dze::optional<int>, dze::test::negative_sentinel<int>)
{
    TestType o1{4};
    TestType o2{42};
    TestType o3;

    SECTION("self")
    {
        REQUIRE(o1 == o1);
        REQUIRE_FALSE(o1 != o1);
        REQUIRE_FALSE(o1 < o1);
        REQUIRE_FALSE(o1 > o1);
        REQUIRE(o1 <= o1);
        REQUIRE(o1 >= o1);
    }

    SECTION("other")
    {
        REQUIRE_FALSE(o1 == o2);
        REQUIRE(o1 != o2);
        REQUIRE(o1 < o2);
        REQUIRE_FALSE(o1 > o2);
        REQUIRE(o1 <= o2);
        REQUIRE_FALSE(o1 >= o2);
    }

    SECTION("nullopt")
    {
        REQUIRE_FALSE(o1 == std::nullopt);
        REQUIRE_FALSE(std::nullopt == o1);
        REQUIRE(o1 != std::nullopt);
        REQUIRE(std::nullopt != o1);
        REQUIRE_FALSE(o1 < std::nullopt);
        REQUIRE(std::nullopt < o1);
        REQUIRE(o1 > std::nullopt);
        REQUIRE_FALSE(std::nullopt > o1);
        REQUIRE_FALSE(o1 <= std::nullopt);
        REQUIRE(std::nullopt <= o1);
        REQUIRE(o1 >= std::nullopt);
        REQUIRE_FALSE(std::nullopt >= o1);

        REQUIRE(o3 == std::nullopt);
        REQUIRE(std::nullopt == o3);
        REQUIRE_FALSE(o3 != std::nullopt);
        REQUIRE_FALSE(std::nullopt != o3);
        REQUIRE_FALSE(o3 < std::nullopt);
        REQUIRE_FALSE(std::nullopt < o3);
        REQUIRE_FALSE(o3 > std::nullopt);
        REQUIRE_FALSE(std::nullopt > o3);
        REQUIRE(o3 <= std::nullopt);
        REQUIRE(std::nullopt <= o3);
        REQUIRE(o3 >= std::nullopt);
        REQUIRE(std::nullopt >= o3);
    }

    SECTION("with T")
    {
        REQUIRE_FALSE(o1 == 1);
        REQUIRE_FALSE(1 == o1);
        REQUIRE(o1 != 1);
        REQUIRE(1 != o1);
        REQUIRE_FALSE(o1 < 1);
        REQUIRE(1 < o1);
        REQUIRE(o1 > 1);
        REQUIRE_FALSE(1 > o1);
        REQUIRE_FALSE(o1 <= 1);
        REQUIRE(1 <= o1);
        REQUIRE(o1 >= 1);
        REQUIRE_FALSE(1 >= o1);

        REQUIRE(o1 == 4);
        REQUIRE(4 == o1);
        REQUIRE_FALSE(o1 != 4);
        REQUIRE_FALSE(4 != o1);
        REQUIRE_FALSE(o1 < 4);
        REQUIRE_FALSE(4 < o1);
        REQUIRE_FALSE(o1 > 4);
        REQUIRE_FALSE(4 > o1);
        REQUIRE(o1 <= 4);
        REQUIRE(4 <= o1);
        REQUIRE(o1 >= 4);
        REQUIRE(4 >= o1);
    }
}

TEMPLATE_TEST_CASE(
    "Relational ops",
    "[relops]",
    dze::optional<std::string>,
    dze::test::empty_sentinel<std::string>)
{
    TestType o1{"hello"};
    TestType o2{"xyz"};
    TestType o3;

    SECTION("self")
    {
        REQUIRE(o1 == o1);
        REQUIRE_FALSE(o1 != o1);
        REQUIRE_FALSE(o1 < o1);
        REQUIRE_FALSE(o1 > o1);
        REQUIRE(o1 <= o1);
        REQUIRE(o1 >= o1);
    }

    SECTION("other")
    {
        REQUIRE_FALSE(o1 == o2);
        REQUIRE(o1 != o2);
        REQUIRE(o1 < o2);
        REQUIRE_FALSE(o1 > o2);
        REQUIRE(o1 <= o2);
        REQUIRE_FALSE(o1 >= o2);
    }

    SECTION("nullopt")
    {
        REQUIRE_FALSE(o1 == std::nullopt);
        REQUIRE_FALSE(std::nullopt == o1);
        REQUIRE(o1 != std::nullopt);
        REQUIRE(std::nullopt != o1);
        REQUIRE_FALSE(o1 < std::nullopt);
        REQUIRE(std::nullopt < o1);
        REQUIRE(o1 > std::nullopt);
        REQUIRE_FALSE(std::nullopt > o1);
        REQUIRE_FALSE(o1 <= std::nullopt);
        REQUIRE(std::nullopt <= o1);
        REQUIRE(o1 >= std::nullopt);
        REQUIRE_FALSE(std::nullopt >= o1);

        REQUIRE(o3 == std::nullopt);
        REQUIRE(std::nullopt == o3);
        REQUIRE_FALSE(o3 != std::nullopt);
        REQUIRE_FALSE(std::nullopt != o3);
        REQUIRE_FALSE(o3 < std::nullopt);
        REQUIRE_FALSE(std::nullopt < o3);
        REQUIRE_FALSE(o3 > std::nullopt);
        REQUIRE_FALSE(std::nullopt > o3);
        REQUIRE(o3 <= std::nullopt);
        REQUIRE(std::nullopt <= o3);
        REQUIRE(o3 >= std::nullopt);
        REQUIRE(std::nullopt >= o3);
    }

    SECTION("with T")
    {
        REQUIRE_FALSE(o1 == "a");
        REQUIRE_FALSE("a" == o1);
        REQUIRE(o1 != "a");
        REQUIRE("a" != o1);
        REQUIRE_FALSE(o1 < "a");
        REQUIRE("a" < o1);
        REQUIRE(o1 > "a");
        REQUIRE_FALSE("a" > o1);
        REQUIRE_FALSE(o1 <= "a");
        REQUIRE("a" <= o1);
        REQUIRE(o1 >= "a");
        REQUIRE_FALSE("a" >= o1);

        REQUIRE(o1 == "hello");
        REQUIRE("hello" == o1);
        REQUIRE_FALSE(o1 != "hello");
        REQUIRE_FALSE("hello" != o1);
        REQUIRE_FALSE(o1 < "hello");
        REQUIRE_FALSE("hello" < o1);
        REQUIRE_FALSE(o1 > "hello");
        REQUIRE_FALSE("hello" > o1);
        REQUIRE(o1 <= "hello");
        REQUIRE("hello" <= o1);
        REQUIRE(o1 >= "hello");
        REQUIRE("hello" >= o1);
    }
}
