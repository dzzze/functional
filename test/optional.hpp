#pragma once

#include <dze/optional.hpp>

namespace q::util::test {

// TODO
using namespace q::util;

template <typename T>
class negative_sentinel_value
{
public:
    constexpr negative_sentinel_value(bool) noexcept {}

    static constexpr bool is_engaged(const T t) noexcept { return t > T{0}; }

    static constexpr T disengaged_initializer() noexcept { return T{-1}; }

    template <typename U>
    static constexpr void set(T& t, U&& u) noexcept
    {
        t = std::forward<U>(u);
    }

    static constexpr void disengage(T& t) noexcept { t = T{-1}; }
};

template <typename T>
using negative_sentinel = ::q::util::optional<T, negative_sentinel_value<T>>;

template <typename T>
class empty_sentinel_value
{
public:
    constexpr empty_sentinel_value(bool) noexcept {}

    [[nodiscard]] static constexpr bool is_engaged(const T& t) noexcept { return !t.empty(); }

    static constexpr T disengaged_initializer() noexcept { return T{}; }

    template <typename U>
    static constexpr void set(T& t, U&& u) noexcept
    {
        t = std::forward<U>(u);
    }

    static constexpr void disengage(T& t) noexcept { t.clear(); }

    static constexpr void cleanup(T& t) noexcept { t.~T(); }
};

template <typename T>
using empty_sentinel = ::q::util::optional<T, empty_sentinel_value<T>>;

} // namespace q::util::test
