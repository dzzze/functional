#include "optional.hpp"

#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch.hpp>

namespace qut = q::util::test;

TEMPLATE_TEST_CASE("Observers", "[observers]", qut::optional<int>, qut::negative_sentinel<int>)
{
    SECTION("Return types")
    {
        TestType o1;
        const TestType o2;

        STATIC_REQUIRE(std::is_same_v<decltype(o1.value()), int&>);
        STATIC_REQUIRE(std::is_same_v<decltype(o2.value()), const int&>);
    }

    SECTION("Basic operation")
    {
        TestType o1 = 42;
        TestType o2;
        const TestType o3 = 42;

        REQUIRE(*o1 == 42);
        REQUIRE(o1.value() == 42);
        REQUIRE(o2.value_or(42) == 42);
        REQUIRE(*o3 == 42);
        REQUIRE(o3.value() == 42);
    }
}

TEMPLATE_TEST_CASE(
    "Observers",
    "[observers]",
    qut::optional<std::vector<int>>,
    qut::empty_sentinel<std::vector<int>>)
{
    SECTION("Return types")
    {
        TestType o1;
        const TestType o2;

        STATIC_REQUIRE(std::is_same_v<decltype(o1.value()), std::vector<int>&>);
        STATIC_REQUIRE(std::is_same_v<decltype(o2.value()), const std::vector<int>&>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::move(o1).value()), std::vector<int>&&>);
        // NOLINTNEXTLINE(performance-move-const-arg)
        STATIC_REQUIRE(
            std::is_same_v<decltype(std::move(o2).value()), const std::vector<int>&&>);
    }
}

TEST_CASE("Observers", "[observers]")
{
    SECTION("Move out")
    {
        struct move_detector
        {
            bool been_moved = false;

            move_detector() = default;

            move_detector(move_detector&& rhs) noexcept { rhs.been_moved = true; }
        };

        qut::optional<move_detector> o4{std::in_place};
        move_detector o5 = std::move(o4).value();
        REQUIRE(o4->been_moved);
        REQUIRE(!o5.been_moved);
    }
}
