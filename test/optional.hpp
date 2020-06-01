#pragma once

#include <util/optional.hpp>

namespace q::util::test {

class default_policy
{
public:
    constexpr default_policy(const bool engaged) noexcept
        : m_engaged{engaged} {}

    [[nodiscard]] constexpr bool is_engaged() const noexcept { return m_engaged; }

    constexpr void set() noexcept { m_engaged = true; }

    constexpr void disengage() noexcept { m_engaged = false; }

private:
    bool m_engaged;
};

// Has equal semantics with std::optional
template <typename T>
using optional = ::q::util::optional<T, default_policy>;

template <typename T>
constexpr auto make_optional(T&& value)
{
    return ::q::util::make_optional<default_policy>(std::forward<T>(value));
}

template <typename T, typename... args_t>
constexpr auto make_optional(args_t&&... args)
{
    return ::q::util::make_optional<T, default_policy>(std::forward<args_t>(args)...);
}

template <typename T, typename U, typename... args_t>
constexpr auto make_optional(std::initializer_list<U> ilist, args_t&&... args)
{
    return ::q::util::make_optional<T, default_policy>(ilist, std::forward<args_t>(args)...);
}

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
