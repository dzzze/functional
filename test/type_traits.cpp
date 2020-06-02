#include "optional.hpp"

#include <type_traits>

#include <catch2/catch.hpp>

TEST_CASE("Triviality", "[type_traits.triviality]")
{
    SECTION("Built-in type")
    {
        STATIC_REQUIRE(std::is_trivially_copy_constructible_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_trivially_copy_assignable_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_trivially_move_constructible_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_trivially_move_assignable_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_trivially_destructible_v<dze::optional<int>>);
    }

    SECTION("Trivial type")
    {
        struct T {};

        STATIC_REQUIRE(std::is_trivially_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_trivially_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_trivially_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_trivially_move_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_trivially_destructible_v<dze::optional<T>>);
    }

    SECTION("Non-trivial type")
    {
        struct T
        {
            // NOLINTNEXTLINE(modernize-use-equals-default)
            T(const T&) {}

            // NOLINTNEXTLINE(modernize-use-equals-default)
            T(T&&) noexcept {}

            // NOLINTNEXTLINE(modernize-use-equals-default)
            T& operator=(const T&) { return *this; }

            // NOLINTNEXTLINE(modernize-use-equals-default)
            T& operator=(T&&) noexcept { return *this; };

            // NOLINTNEXTLINE(modernize-use-equals-default)
            ~T() {}
        };

        STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_trivially_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_trivially_move_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_trivially_destructible_v<dze::optional<T>>);
    }
}

TEST_CASE("Deletion", "[type_traits.deletion]")
{
    SECTION("Built-in type")
    {
        STATIC_REQUIRE(std::is_copy_constructible_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_copy_assignable_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_move_constructible_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_move_assignable_v<dze::optional<int>>);
        STATIC_REQUIRE(std::is_destructible_v<dze::optional<int>>);
    }

    SECTION("Trivial type")
    {
        struct T
        {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };

        STATIC_REQUIRE(std::is_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_destructible_v<dze::optional<T>>);
    }

    SECTION("All deleted")
    {
        struct T
        {
            T(const T&) = delete;
            T(T&&) = delete;
            T& operator=(const T&) = delete;
            T& operator=(T&&) = delete;
        };

        STATIC_REQUIRE(!std::is_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_move_assignable_v<dze::optional<T>>);
    }

    SECTION("Movable-only type")
    {
        struct T
        {
            T(const T&) = delete;
            T(T&&) = default;
            T& operator=(const T&) = delete;
            T& operator=(T&&) = default;
        };

        STATIC_REQUIRE(!std::is_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(!std::is_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_assignable_v<dze::optional<T>>);
    }

    SECTION("Copiable-only type")
    {
        struct T
        {
            T(const T&) = default;
            T(T&&) = delete;
            T& operator=(const T&) = default;
            T& operator=(T&&) = delete;
        };

        STATIC_REQUIRE(std::is_copy_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_copy_assignable_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_constructible_v<dze::optional<T>>);
        STATIC_REQUIRE(std::is_move_assignable_v<dze::optional<T>>);
    }
}
