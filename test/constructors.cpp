#include "optional.hpp"

#include <utility>
#include <vector>

#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE(
    "Constructors", "[constructors]", dze::optional<int>, dze::test::negative_sentinel<int>)
{
    SECTION("empty construct")
    {
        constexpr TestType o1;
        constexpr TestType o2{};
        constexpr TestType o3 = {};
        constexpr TestType o4 = std::nullopt;
        constexpr TestType o5 = {std::nullopt};
        constexpr TestType o6(std::nullopt);

        STATIC_REQUIRE(!o1);
        STATIC_REQUIRE(!o2);
        STATIC_REQUIRE(!o3);
        STATIC_REQUIRE(!o4);
        STATIC_REQUIRE(!o5);
        STATIC_REQUIRE(!o6);
    }

    SECTION("value construct")
    {
        constexpr TestType o1 = 42;
        constexpr TestType o2{42};
        constexpr TestType o3(42);
        constexpr TestType o4 = {42};

        constexpr int i = 42;
        constexpr TestType o5 = i;
        constexpr TestType o6{i};
        constexpr TestType o7(i);
        constexpr TestType o8 = {i};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
        STATIC_REQUIRE(*o5 == 42);
        STATIC_REQUIRE(*o6 == 42);
        STATIC_REQUIRE(*o7 == 42);
        STATIC_REQUIRE(*o8 == 42);
    }

    SECTION("construct from another")
    {
        constexpr TestType o1 = 42;
        constexpr TestType o2 = o1;
        constexpr TestType o3{o1};
        constexpr TestType o4(o1);
        constexpr TestType o5 = {o1};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
        STATIC_REQUIRE(*o5 == 42);
    }

    SECTION("construct from another optional type")
    {
        dze::optional<short> o1 = 42;
        TestType o2 = o1;
        TestType o3{o1};
        TestType o4(o1);
        TestType o5 = {o1};

        CHECK(*o1 == 42);
        CHECK(*o2 == 42);
        CHECK(*o3 == 42);
        CHECK(*o4 == 42);
        CHECK(*o5 == 42);
    }
}

struct foo
{
    foo() = default;
    foo(const foo&) = delete;
    foo(foo&&) noexcept {}; // NOLINT(modernize-use-equals-default)
};

TEMPLATE_TEST_CASE(
    "Constructors",
    "[constructors]",
    dze::optional<std::vector<foo>>,
    dze::test::empty_sentinel<std::vector<foo>>)
{
    std::vector<foo> v;
    v.emplace_back();
    TestType ov1 = std::move(v);
    REQUIRE(v.empty());
    REQUIRE(ov1.has_value());
    REQUIRE(ov1->size() == 1);

    TestType ov2 = std::move(ov1);
    REQUIRE(ov2.has_value());
    REQUIRE(ov2->size() == 1);
    if constexpr (std::is_same_v<TestType, dze::optional<std::vector<foo>>>)
    {
        REQUIRE(ov1.has_value());
        CHECK(ov1->empty());
    }
    else
        CHECK(!ov1.has_value());
}
