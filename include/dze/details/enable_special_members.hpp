#pragma once

#include <type_traits>

namespace dze::details {

struct enable_default_constructor_tag
{
    explicit constexpr enable_default_constructor_tag() noexcept = default;
};

// A mixin helper to conditionally enable or disable the default constructor.
template <bool, typename = void>
struct enable_default_constructor
{
    constexpr enable_default_constructor() noexcept = default;
    explicit constexpr enable_default_constructor(enable_default_constructor_tag) noexcept {}
    constexpr enable_default_constructor(const enable_default_constructor&) noexcept = default;
    enable_default_constructor& operator=(
        const enable_default_constructor&) noexcept = default;
    constexpr enable_default_constructor(enable_default_constructor&&) noexcept = default;
    enable_default_constructor& operator=(enable_default_constructor&&) noexcept = default;
};

template <typename T>
using default_constructible_base =
    enable_default_constructor<std::is_default_constructible_v<T>, T>;

// A mixin helper to conditionally enable or disable the destructor.
template <bool, typename = void>
struct enable_destructor {};

template <typename T>
using destructible_base = enable_destructor<std::is_destructible_v<T>, T>;

// A mixin helper to conditionally enable or disable the destructor.
template <
    bool CopyConstructible,
    bool CopyAssignable,
    bool MoveConstructible,
    bool MoveAssignable,
    typename = void>
struct enable_copy_move {};

template <typename T>
using copy_move_base = enable_copy_move<
    std::is_copy_constructible_v<T>,
    std::is_copy_assignable_v<T>,
    std::is_move_constructible_v<T>,
    std::is_move_assignable_v<T>,
    T>;

template <
    bool DefConstrutible,
    bool Destructible,
    bool CopyConstructible,
    bool CopyAssignable,
    bool MoveConstructible,
    bool MoveAssignable,
    typename Tag = void>
struct enable_special_members
    : private enable_default_constructor<DefConstrutible, Tag>
    , private enable_destructor<Destructible, Tag>
    , private enable_copy_move<
        CopyConstructible, CopyAssignable, MoveConstructible, MoveAssignable, Tag> {};

template <typename T>
using special_members_base = enable_special_members<
    std::is_default_constructible_v<T>,
    std::is_destructible_v<T>,
    std::is_copy_constructible_v<T>,
    std::is_copy_assignable_v<T>,
    std::is_move_constructible_v<T>,
    std::is_move_assignable_v<T>,
    T>;

// Boilerplate follows.

template <typename Tag>
struct enable_default_constructor<false, Tag>
{
    constexpr enable_default_constructor() noexcept = delete;
    explicit constexpr enable_default_constructor(enable_default_constructor_tag) noexcept {}
    constexpr enable_default_constructor(const enable_default_constructor&) noexcept = default;
    enable_default_constructor& operator=(
        const enable_default_constructor&) noexcept = default;
    constexpr enable_default_constructor(enable_default_constructor&&) noexcept = default;
    enable_default_constructor& operator=(enable_default_constructor&&) noexcept = default;
};

template <typename Tag>
struct enable_destructor<false, Tag>
{
    ~enable_destructor() noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<false, false, false, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<false, false, false, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<false, false, true, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<false, false, true, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<false, true, false, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<false, true, false, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<false, true, true, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<false, true, true, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<true, false, false, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<true, false, false, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<true, false, true, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<true, false, true, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<true, true, false, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <typename Tag>
struct enable_copy_move<true, true, false, true, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <typename Tag>
struct enable_copy_move<true, true, true, false, Tag>
{
    constexpr enable_copy_move() noexcept = default;
    constexpr enable_copy_move(const enable_copy_move&) noexcept = default;
    enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
    constexpr enable_copy_move(enable_copy_move&&) noexcept = default;
    enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

} // namespace dze::details
