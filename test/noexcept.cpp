#include "optional.hpp"

#include <optional>
#include <type_traits>

#include <catch2/catch.hpp>

TEST_CASE("Noexcept", "[noexcept]")
{
    dze::optional<int> o1{4};
    dze::optional<int> o2{42};

    SECTION("comparison with nullopt")
    {
        STATIC_REQUIRE(noexcept(o1 == std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt == o1));
        STATIC_REQUIRE(noexcept(o1 != std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt != o1));
        STATIC_REQUIRE(noexcept(o1 < std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt < o1));
        STATIC_REQUIRE(noexcept(o1 <= std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt <= o1));
        STATIC_REQUIRE(noexcept(o1 > std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt > o1));
        STATIC_REQUIRE(noexcept(o1 >= std::nullopt));
        STATIC_REQUIRE(noexcept(std::nullopt >= o1));
    }

    SECTION("swap")
    {
        STATIC_REQUIRE(noexcept(swap(o1, o2)) == noexcept(o1.swap(o2)));

        struct nothrow_swappable
        {
            nothrow_swappable& swap(const nothrow_swappable&) noexcept { return *this; }
        };

        struct throw_swappable
        {
            throw_swappable() = default;

            // NOLINTNEXTLINE(modernize-use-equals-default)
            throw_swappable(const throw_swappable&) {}

            // NOLINTNEXTLINE(modernize-use-equals-default)
            throw_swappable& swap(const throw_swappable&) { return *this; }

            // NOLINTNEXTLINE(modernize-use-equals-default, performance-noexcept-move-constructor)
            throw_swappable(throw_swappable&&) {}
        };

        dze::optional<nothrow_swappable> ont;
        dze::optional<throw_swappable> ot;

        STATIC_REQUIRE(noexcept(ont.swap(ont)));
        STATIC_REQUIRE(!noexcept(ot.swap(ot)));
    }

    SECTION("constructors")
    {
        STATIC_REQUIRE(noexcept(dze::optional<int>{}));
        STATIC_REQUIRE(noexcept(dze::optional<int>{std::nullopt}));

        struct nothrow_move
        {
            nothrow_move(nothrow_move&&) noexcept = default;
        };

        struct throw_move
        {
            // NOLINTNEXTLINE(modernize-use-equals-default, performance-noexcept-move-constructor)
            throw_move(throw_move&&) {};
        };

        using nothrow_opt = dze::optional<nothrow_move>;
        using throw_opt = dze::optional<throw_move>;

        STATIC_REQUIRE(std::is_nothrow_move_constructible<nothrow_opt>::value);
        STATIC_REQUIRE(!std::is_nothrow_move_constructible<throw_opt>::value);
    }

    SECTION("assignment")
    {
        STATIC_REQUIRE(noexcept(o1 = std::nullopt));

        struct nothrow_move_assign
        {
            nothrow_move_assign() = default;
            nothrow_move_assign(nothrow_move_assign&&) noexcept = default;
            nothrow_move_assign& operator=(const nothrow_move_assign&) = default;
        };

        struct throw_move_assign
        {
            throw_move_assign() = default;

            // NOLINTNEXTLINE(modernize-use-equals-default)
            throw_move_assign& operator=(const throw_move_assign&) { return *this; }

            // NOLINTNEXTLINE(modernize-use-equals-default, performance-noexcept-move-constructor)
            throw_move_assign(throw_move_assign&&) {};
        };

        using nothrow_opt = dze::optional<nothrow_move_assign>;
        using throw_opt = dze::optional<throw_move_assign>;

        STATIC_REQUIRE(noexcept(std::declval<nothrow_opt>() = std::declval<nothrow_opt>()));
        STATIC_REQUIRE(!noexcept(std::declval<throw_opt>() = std::declval<throw_opt>()));
    }

    SECTION("observers")
    {
        STATIC_REQUIRE(noexcept(static_cast<bool>(o1)));
        STATIC_REQUIRE(noexcept(o1.has_value()));
    }

    SECTION("modifiers") { STATIC_REQUIRE(noexcept(o1.reset())); }
}
