#include "optional.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE(
    "Assignment", "[assignment]", dze::optional<int>, dze::test::negative_sentinel<int>)
{
    TestType o1 = 42;

    SECTION("Assign value")
    {
        o1 = 84;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Assign value of different type")
    {
        o1 = short{84};
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Self assign")
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == 42);
    }

    SECTION("Assign another optional")
    {
        TestType o2 = 84;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional")
    {
        TestType o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        dze::optional<short> o2 = 84;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional of different type")
    {
        dze::optional<short> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}

TEMPLATE_TEST_CASE(
    "Assignment", "[assignment]", dze::optional<std::string>, dze::test::empty_sentinel<std::string>)
{
    TestType o1 = std::string{"42"};

    SECTION("Assign value")
    {
        o1 = std::string{"84"};
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
    }

    SECTION("Assign value of different type")
    {
        o1 = "84";
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
    }

    SECTION("Self assign")
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == "42");
    }

    SECTION("Assign another optional")
    {
        TestType o2 = std::string{"84"};
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        REQUIRE(o2.has_value());
        CHECK(*o2 == "84");
    }

    SECTION("Move assign another optional")
    {
        TestType o2 = std::string{"84"};
        o1 = std::move(o2);
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        if constexpr (std::is_same_v<TestType, dze::optional<std::string>>)
        {
            REQUIRE(o2.has_value());
            CHECK(o2->empty());
        }
        else
            CHECK(!o2.has_value());
    }

    SECTION("Assign empty optional")
    {
        TestType o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Move assign empty optional")
    {
        TestType o2;
        o1 = std::move(o2);
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        dze::optional<const char*> o2 = "84";
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        REQUIRE(o2.has_value());
        CHECK(*o2 == std::string_view{"84"});
    }

    SECTION("Assign empty optional of different type")
    {
        dze::optional<const char*> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}
