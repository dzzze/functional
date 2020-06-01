#pragma once

#include <cassert>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

#include "type_traits.hpp"

#include "details/enable_special_members.hpp"

namespace q::util {

namespace details {

template <typename Policy>
using disengaged_initializer_t = decltype(std::declval<Policy>().disengaged_initializer());

template <typename Policy>
constexpr bool has_disengaged_initializer_v = is_detected_v<disengaged_initializer_t, Policy>;

// This class template manages construction/destruction of
// the contained value for a util::optional.
template <typename T, typename Policy>
class optional_payload_base : private Policy
{
    using stored_type = std::remove_const_t<T>;

public:
    template <typename dummy = Policy,
        Q_UTIL_REQUIRES(!has_disengaged_initializer_v<dummy>)>
    constexpr optional_payload_base() noexcept
        : Policy{false} {}

    template <typename dummy = Policy,
        Q_UTIL_REQUIRES(has_disengaged_initializer_v<dummy>)>
    constexpr optional_payload_base() noexcept
        : Policy{false}
        , m_payload{std::in_place, static_cast<Policy&>(*this).disengaged_initializer()} {}

    template <typename... Args>
    constexpr optional_payload_base(std::in_place_t, Args&&... args)
        : Policy{true}
        , m_payload{std::in_place, std::forward<Args>(args)...}
    {
        assert(is_engaged());
    }

    template <typename U, typename... Args>
    constexpr optional_payload_base(std::initializer_list<U> ilist, Args&&... args)
        : Policy{true}
        , m_payload{ilist, std::forward<Args>(args)...}
    {
        assert(is_engaged());
    }

    // Constructor used by optional_base copy constructor when the
    // contained value is not trivially copy constructible.
    constexpr optional_payload_base(bool, const optional_payload_base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : Policy{other.is_engaged()}
    {
        if (other.is_engaged())
            construct(other.get());
    }

    // Constructor used by optional_base move constructor when the
    // contained value is not trivially move constructible.
    constexpr optional_payload_base(bool, optional_payload_base&& other)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        : Policy{other.is_engaged()}
    {
        if (other.is_engaged())
            construct(std::move(other.get()));
    }

    template <typename... Args>
    void construct(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        ::new
            (static_cast<void*>(std::addressof(m_payload.m_value)))
            stored_type(std::forward<Args>(args)...);
    }

    template <typename dummy = Policy, typename... Args,
        Q_UTIL_REQUIRES(has_disengaged_initializer_v<dummy>)>
    void set(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        static_cast<Policy*>(this)->set(m_payload.m_value, std::forward<Args>(args)...);
    }

    template <typename dummy = Policy,
        Q_UTIL_REQUIRES(!has_disengaged_initializer_v<dummy>)>
    void set() noexcept
    {
        static_cast<Policy*>(this)->set();
    }

    constexpr void copy_assign(const optional_payload_base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
    {
        if (is_engaged() && other.is_engaged())
            get() = other.get();
        else if (other.is_engaged())
        {
            if constexpr (has_disengaged_initializer_v<Policy>)
                set(other.get());
            else
                construct(other.get());
        }
        else
            reset();
    }

    constexpr void move_assign(optional_payload_base&& other)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
    {
        if (is_engaged() && other.is_engaged())
            get() = std::move(other.get());
        else if (other.is_engaged())
        {
            if constexpr (has_disengaged_initializer_v<Policy>)
                set(std::move(other.get()));
            else
                construct(std::move(other.get()));
        }
        else
            reset();
    }

    // Policies are responsible for cleaning up disengaged objects.
    // While reset puts the managed object to null state, destruct actually
    // destroys it.
    constexpr void destruct() noexcept
    {
        if (is_engaged())
            get().~stored_type();
        else if constexpr (
            !std::is_trivially_destructible_v<T> && has_disengaged_initializer_v<Policy>)
        {
            static_cast<Policy*>(this)->cleanup(m_payload.m_value);
        }
    }

    [[nodiscard]] constexpr bool is_engaged() const noexcept
    {
        if constexpr (has_disengaged_initializer_v<Policy>)
            return static_cast<const Policy*>(this)->is_engaged(m_payload.m_value);
        else
            return static_cast<const Policy*>(this)->is_engaged();
    }

    constexpr void unchecked_reset() noexcept
    {
        if constexpr (has_disengaged_initializer_v<Policy>)
            static_cast<Policy*>(this)->disengage(get());
        else
        {
            get().~stored_type();
            static_cast<Policy*>(this)->disengage();
        }
    }

    // reset is a 'safe' operation with no precondition.
    constexpr void reset() noexcept
    {
        if (is_engaged())
            unchecked_reset();
    }

    // The get() operations have is_engaged() as a precondition.
    // They exist to access the contained value with the appropriate
    // const-qualification, because m_payload has had the const removed.
    [[nodiscard]] constexpr const T& get() const noexcept
    {
        assert(is_engaged());

        return m_payload.m_value;
    }

    [[nodiscard]] constexpr T& get() noexcept
    {
        assert(is_engaged());

        return m_payload.m_value;
    }

private:
    struct empty_byte {};

    template <typename U, bool = std::is_trivially_destructible_v<U>>
    union storage
    {
        constexpr storage() noexcept
            : m_empty{} {}

        // Not using brace initializers here to allow potential narrowing conversions.

        template <typename... Args>
        constexpr storage(std::in_place_t, Args&&... args)
            : m_value(std::forward<Args>(args)...) {}

        template <typename V, typename... Args>
        constexpr storage(std::initializer_list<V> ilist, Args&&... args)
            : m_value(ilist, std::forward<Args>(args)...) {}

        empty_byte m_empty;
        stored_type m_value;
    };

    template <typename U>
    union storage<U, false>
    {
        constexpr storage() noexcept
            : m_empty{} {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename... Args>
        constexpr storage(std::in_place_t, Args&&... args)
            : m_value(std::forward<Args>(args)...) {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename V, typename... Args>
        constexpr storage(std::initializer_list<V> ilist, Args&&... args)
            : m_value(ilist, std::forward<Args>(args)...) {}

        // User-provided destructor is needed when U has a non-trivial dtor.
        // Clang Tidy does not understand non-default destuctors of unions.
        // NOLINTNEXTLINE(modernize-use-equals-default)
        ~storage() {}

        empty_byte m_empty;
        stored_type m_value;
    };

    // NOLINTNEXTLINE(modernize-use-default-member-init)
    storage<stored_type> m_payload;
};

// Class template that manages the payload for optionals.
template <
    typename T,
    typename Policy,
    bool = std::is_trivially_destructible_v<T>,
    bool = std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>,
    bool = std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>>
class optional_payload;

// Payload for potentially-constexpr optionals.
template <typename T, typename Policy>
class optional_payload<T, Policy, true, true, true> : public optional_payload_base<T, Policy>
{
public:
    using optional_payload_base<T, Policy>::optional_payload_base;

    optional_payload() = default;
};

// Clang-Tidy generates incorrect warnings for defaulted move constructors/assignment
// operators.

// Payload for optionals with non-trivial copy construction/assignment.
template <typename T, typename Policy>
class optional_payload<T, Policy, true, false, true> : public optional_payload_base<T, Policy>
{
public:
    using optional_payload_base<T, Policy>::optional_payload_base;

    optional_payload() = default;

    optional_payload(const optional_payload&) = default;

    constexpr optional_payload& operator=(const optional_payload& other)
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
    {
        this->copy_assign(other);
        return *this;
    }

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload(optional_payload&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload& operator=(optional_payload&&) = default;
};

// Payload for optionals with non-trivial move construction/assignment.
template <typename T, typename Policy>
class optional_payload<T, Policy, true, true, false> : public optional_payload_base<T, Policy>
{
public:
    using optional_payload_base<T, Policy>::optional_payload_base;

    optional_payload() = default;

    optional_payload(const optional_payload&) = default;

    optional_payload& operator=(const optional_payload&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload(optional_payload&&) = default;

    constexpr optional_payload& operator=(optional_payload&& other)
        // This class assumes the movability of the contained type.
        // Therefore, noexcept specifiers should be applied to the
        // contained type.
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
    {
        this->move_assign(std::move(other));
        return *this;
    }
};

// Payload for optionals with non-trivial copy and move assignment.
template <typename T, typename Policy>
class optional_payload<T, Policy, true, false, false> : public optional_payload_base<T, Policy>
{
public:
    using optional_payload_base<T, Policy>::optional_payload_base;

    optional_payload() = default;

    optional_payload(const optional_payload&) = default;

    constexpr optional_payload& operator=(const optional_payload& other)
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
    {
        this->copy_assign(other);
        return *this;
    }

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload(optional_payload&&) = default;

    constexpr optional_payload& operator=(optional_payload&& other)
        // This class assumes the movability of the contained type.
        // Therefore, noexcept specifiers should be applied to the
        // contained type.
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
    {
        this->move_assign(std::move(other));
        return *this;
    }
};

// Payload for optionals with non-trivial destructors.
template <typename T, typename Policy, bool copy, bool move>
class optional_payload<T, Policy, false, copy, move>
    : public optional_payload<T, Policy, true, copy, move>
{
public:
    // Base class implements all the constructors and assignment operators:
    using optional_payload<T, Policy, true, copy, move>::optional_payload;

    optional_payload() = default;
    optional_payload(const optional_payload&) = default;
    optional_payload& operator=(const optional_payload&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload(optional_payload&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_payload& operator=(optional_payload&&) = default;

    // Destructor needs to destroy the contained value.
    ~optional_payload() { this->destruct(); }
};

// Common base class for optional_base<T, Policy> to avoid repeating these
// member functions in each specialization.
template <typename T, typename Policy, typename optional_base>
class optional_base_impl
{
protected:
    using stored_type = std::remove_const_t<T>;

    // construct has !is_engaged() as a precondition.
    // Should be used when a new optional object is being constructed.
    template <typename... Args>
    void construct(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        auto& payload = static_cast<optional_base*>(this)->payload();
        payload.construct(std::forward<Args>(args)...);
        if constexpr (!has_disengaged_initializer_v<Policy>)
            payload.set();
    }

    // set has !is_engaged() as a precondition.
    // Should be used on an existing optional object.
    template <typename... Args>
    void set(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        auto& payload = static_cast<optional_base*>(this)->payload();
        if constexpr (has_disengaged_initializer_v<Policy>)
            payload.set(std::forward<Args>(args)...);
        else
        {
            payload.construct(std::forward<Args>(args)...);
            payload.set();
        }
    }

    // destruct has is_engaged() as a precondition.
    void destruct() noexcept { static_cast<optional_base*>(this)->payload().destruct(); }

    [[nodiscard]] constexpr bool is_engaged() const noexcept
    {
        return static_cast<const optional_base*>(this)->payload().is_engaged();
    }

    // unchecked_reset has is_engaged() as a precondition.
    constexpr void unchecked_reset() noexcept
    {
        static_cast<optional_base*>(this)->payload().unchecked_reset();
    }

    // reset is a 'safe' operation with no precondition.
    constexpr void reset_impl() noexcept
    {
        static_cast<optional_base*>(this)->payload().reset();
    }

    // The get() operations have is_engaged as a precondition.

    [[nodiscard]] constexpr const T& get() const noexcept
    {
        assert(is_engaged());
        return static_cast<const optional_base*>(this)->payload().get();
    }

    [[nodiscard]] constexpr T& get() noexcept
    {
        assert(is_engaged());
        return static_cast<optional_base*>(this)->payload().get();
    }
};

/*
Class template that takes care of copy/move constructors of optional.

Such a separate base class template is necessary in order to
conditionally make copy/move constructors trivial.

When the contained value is trivally copy/move constructible,
the copy/move constructors of optional_base will invoke the
trivial copy/move constructor of optional_payload. Otherwise,
they will invoke optional_payload(bool, const optional_payload&)
or optional_payload(bool, optional_payload&&) to initialize
the contained value, if copying/moving an engaged optional.

Whether the other special members are trivial is determined by the
optional_payload<T> specialization used for the m_payload member.
*/
template <
    typename T,
    typename Policy,
    bool = std::is_trivially_copy_constructible_v<T>,
    bool = std::is_trivially_move_constructible_v<T>>
class optional_base : public optional_base_impl<T, Policy, optional_base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    constexpr optional_base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional_base(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr optional_base(const optional_base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : m_payload{other.m_payload.is_engaged(), other.m_payload} {}

    optional_base& operator=(const optional_base&) = default;

    constexpr optional_base(optional_base&& other)
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_payload{other.m_payload.is_engaged(), std::move(other.m_payload)} {}

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_base& operator=(optional_base&&) = default;

    [[nodiscard]] constexpr auto& payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& payload() noexcept { return m_payload; }

private:
    optional_payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class optional_base<T, Policy, false, true>
    : public optional_base_impl<T, Policy, optional_base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    constexpr optional_base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional_base(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr optional_base(const optional_base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : m_payload{other.m_payload.is_engaged(), other.m_payload} {}

    optional_base& operator=(const optional_base&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    constexpr optional_base(optional_base&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_base& operator=(optional_base&&) = default;

    [[nodiscard]] constexpr auto& payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& payload() noexcept { return m_payload; }

private:
    optional_payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class optional_base<T, Policy, true, false>
    : public optional_base_impl<T, Policy, optional_base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    constexpr optional_base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr optional_base(const optional_base&) = default;

    optional_base& operator=(const optional_base&) = default;

    constexpr optional_base(optional_base&& other)
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_payload{other.m_payload.is_engaged(), std::move(other.m_payload)} {}

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    optional_base& operator=(optional_base&&) = default;

    [[nodiscard]] constexpr auto& payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& payload() noexcept { return m_payload; }

private:
    optional_payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class optional_base<T, Policy, true, true>
    : public optional_base_impl<T, Policy, optional_base<T, Policy>>
{
public:
    // Constructors for disengaged optionals.
    constexpr optional_base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional_base(std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    [[nodiscard]] constexpr auto& payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& payload() noexcept { return m_payload; }

private:
    optional_payload<T, Policy> m_payload;
};

template <typename T>
using optional_enable_copy_move = enable_copy_move<
    std::is_copy_constructible_v<T>,
    std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>,
    std::is_move_constructible_v<T>,
    std::is_move_constructible_v<T> && std::is_move_assignable_v<T>,
    T>;

} // namespace details

template <typename T, typename Policy>
class optional;

namespace details {

template <typename T, typename U, typename Policy>
constexpr bool converts_from_optional =
    std::is_constructible_v<T, const util::optional<U, Policy>&> ||
    std::is_constructible_v<T, util::optional<U, Policy>&> ||
    std::is_constructible_v<T, const util::optional<U, Policy>&&> ||
    std::is_constructible_v<T, util::optional<U, Policy>&&> ||
    std::is_convertible_v<const util::optional<U, Policy>&, T> ||
    std::is_convertible_v<util::optional<U, Policy>&, T> ||
    std::is_convertible_v<const util::optional<U, Policy>&&, T> ||
    std::is_convertible_v<util::optional<U, Policy>&&, T>;

template <typename T, typename U, typename Policy>
constexpr bool assigns_from_optional =
    std::is_assignable_v<T&, const util::optional<U, Policy>&> ||
    std::is_assignable_v<T&, util::optional<U, Policy>&> ||
    std::is_assignable_v<T&, const util::optional<U, Policy>&&> ||
    std::is_assignable_v<T&, util::optional<U, Policy>&&>;

} // namespace details

// This policy provides equivalent semantics with std::optional.
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

template <typename T, typename Policy = default_policy>
class optional
    : private details::optional_base<T, Policy>
    , private details::optional_enable_copy_move<T>
{
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::nullopt_t>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>);
    static_assert(!std::is_reference_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<Policy>);
    static_assert(std::is_trivially_move_constructible_v<Policy>);
    static_assert(std::is_trivially_copy_assignable_v<Policy>);
    static_assert(std::is_trivially_move_assignable_v<Policy>);
    static_assert(std::is_trivially_destructible_v<Policy>);

    using base = details::optional_base<T, Policy>;

public:
    using value_type = T;

    constexpr optional() = default;

    constexpr optional(std::nullopt_t) noexcept {}

    // Converting constructors for engaged optionals.
    template <typename U = T,
        Q_UTIL_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            !std::is_same_v<std::in_place_t, std::decay_t<U>> &&
            std::is_constructible_v<T, U&&> &&
            std::is_convertible_v<U&&, T>)>
    constexpr optional(U&& t)
        : base{std::in_place, std::forward<U>(t)} {}

    template <typename U = T,
        Q_UTIL_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            !std::is_same_v<std::in_place_t, std::decay_t<U>> &&
            std::is_constructible_v<T, U&&> &&
            !std::is_convertible_v<U&&, T>)>
    explicit constexpr optional(U&& t)
        : base{std::in_place, std::forward<U>(t)} {}

    template <typename U, typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            std::is_convertible_v<const U&, T> &&
            !details::converts_from_optional<T, U, Policy2>)>
    constexpr optional(const optional<U, Policy2>& other)
    {
        if (other)
            emplace(*other);
    }

    template <typename U, typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            !std::is_convertible_v<const U&, T> &&
            !details::converts_from_optional<T, U, Policy2>)>
    explicit constexpr optional(const optional<U, Policy2>& other)
    {
        if (other)
            emplace(*other);
    }

    template <typename U, typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            std::is_convertible_v<U&&, T> &&
            !details::converts_from_optional<T, U, Policy2>)>
    constexpr optional(optional<U, Policy2>&& other)
    {
        if (other)
            emplace(std::move(*other));
    }

    template <typename U, typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            !std::is_convertible_v<U&&, T> &&
            !details::converts_from_optional<T, U, Policy2>)>
    explicit constexpr optional(optional<U, Policy2>&& other)
    {
        if (other)
            emplace(std::move(*other));
    }

    template <typename... Args,
             Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional(std::in_place_t, Args&&... args)
        : base{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : base{std::in_place, ilist, std::forward<Args>(args)...} {}

    optional& operator=(std::nullopt_t) noexcept
    {
        this->reset();
        return *this;
    }

    template <typename U = T,
        Q_UTIL_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            std::is_constructible_v<T, U> &&
            !(std::is_scalar_v<T> &&
            std::is_same_v<T, std::decay_t<U>>) &&
            std::is_assignable_v<T&, U>)>
    optional& operator=(U&& value)
    {
        if (this->is_engaged())
            this->get() = std::forward<U>(value);
        else
            this->set(std::forward<U>(value));

        return *this;
    }

    template <
        typename U,
        typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            std::is_assignable_v<T&, const U&> &&
            !details::converts_from_optional<T, U, Policy2> &&
            !details::assigns_from_optional<T, U, Policy2>)>
    optional& operator=(const optional<U, Policy2>& other)
    {
        if (other)
        {
            if (this->is_engaged())
                this->get() = *other;
            else
                this->set(*other);
        }
        else
            this->reset();

        return *this;
    }

    template <typename U, typename Policy2,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            std::is_assignable_v<T&, U&&> &&
            !details::converts_from_optional<T, U, Policy2> &&
            !details::assigns_from_optional<T, U, Policy2>)>
    optional& operator=(optional<U, Policy2>&& other)
    {
        if (other)
        {
            if (this->is_engaged())
                this->get() = std::move(*other);
            else
                this->set(std::move(*other));
        }
        else
            this->reset();

        return *this;
    }

    template <typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    T& emplace(Args&&... args)
    {
        this->destruct();
        this->construct(std::forward<Args>(args)...);
        return this->get();
    }

    template <typename U, typename... Args,
        Q_UTIL_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>, Args&&...>)>
    T& emplace(std::initializer_list<U> ilist, Args&&... args)
    {
        this->destruct();
        this->construct(ilist, std::forward<Args>(args)...);
        return this->get();
    }

    // Destructor is implicit, implemented in optional_base.

    void swap(optional& other)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>)
    {
        using std::swap;

        if (this->is_engaged() && other.is_engaged())
            swap(this->get() && other.get());
        else if (this->is_engaged())
        {
            other.set(std::move(this->get()));
            this->unchecked_reset();
        }
        else if (other.is_engaged())
        {
            this->set(std::move(other.get()));
            other.unchecked_reset();
        }
    }

    constexpr const T* operator->() const { return std::addressof(this->get()); }

    constexpr T* operator->() { return std::addressof(this->get()); }

    constexpr const T& operator*() const& { return this->get(); }

    constexpr T& operator*() & { return this->get(); }

    constexpr const T&& operator*() const&& { return std::move(this->get()); }

    constexpr T&& operator*() && { return std::move(this->get()); }

    explicit constexpr operator bool() const noexcept { return this->is_engaged(); }

    [[nodiscard]] constexpr bool has_value() const noexcept { return this->is_engaged(); }

    [[nodiscard]] constexpr T& value() &
    {
        return this->is_engaged()
            ? this->get()
            : (throw std::bad_optional_access{}, this->get());
    }

    [[nodiscard]] constexpr const T& value() const&
    {
        return this->is_engaged()
            ? this->get()
            : (throw std::bad_optional_access{}, this->get());
    }

    [[nodiscard]] constexpr T&& value() &&
    {
        return this->is_engaged()
            ? std::move(this->get())
            : (throw std::bad_optional_access{}, std::move(this->get()));
    }

    [[nodiscard]] constexpr const T&& value() const&&
    {
        return this->is_engaged()
            ? std::move(this->get())
            : (throw std::bad_optional_access{}, std::move(this->get()));
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& default_value) const&
    {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return this->is_engaged()
            ? this->get()
            : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& default_value) &&
    {
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return this->is_engaged()
            ? std::move(this->get())
            : static_cast<T>(std::forward<U>(default_value));
    }

    void reset() noexcept { this->reset_impl(); }
};

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) == static_cast<bool>(rhs) && (!lhs || *lhs == *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) != static_cast<bool>(rhs) ||
        (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs > *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs <= *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs >= *rhs);
}

// Comparisons with nullopt.
template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator==(
    const optional<T, Policy>& lhs, std::nullopt_t) noexcept
{
    return !lhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator==(
    std::nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return !rhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator!=(
    const optional<T, Policy>& lhs, std::nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator!=(
    std::nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<(const optional<T, Policy>&, std::nullopt_t) noexcept
{
    return false;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<(std::nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>(const optional<T, Policy>& lhs, std::nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>(std::nullopt_t, const optional<T, Policy>&) noexcept
{
    return false;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<=(
    const optional<T, Policy>& lhs, std::nullopt_t) noexcept
{
    return !lhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<=(std::nullopt_t, const optional<T, Policy>&) noexcept
{
    return true;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>=(const optional<T, Policy>&, std::nullopt_t) noexcept
{
    return true;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>=(
    std::nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return !rhs;
}

// Comparisons with value type.

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs == rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs == *rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs != rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs != *rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs < rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs < *rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs > rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs > *rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs <= rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs <= *rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs >= rhs;
}

template <typename T, typename Policy, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs >= *rhs;
}

template <typename T, typename Policy,
    Q_UTIL_REQUIRES(std::is_move_constructible_v<T> && std::is_swappable_v<T>)>
void swap(optional<T, Policy>& lhs, optional<T, Policy>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

template <typename T, typename Policy,
    Q_UTIL_REQUIRES(!(std::is_move_constructible_v<T> && std::is_swappable_v<T>))>
void swap(optional<T, Policy>&, optional<T, Policy>&) = delete;

template <typename T>
[[nodiscard]] constexpr optional<std::decay_t<T>> make_optional(T&& value)
{
    return optional<std::decay_t<T>>{std::forward<T>(value)};
}

template <typename Policy, typename T>
[[nodiscard]] constexpr optional<std::decay_t<T>, Policy> make_optional(T&& value)
{
    return optional<std::decay_t<T>, Policy>{std::forward<T>(value)};
}

template <typename T, typename... Args>
[[nodiscard]] constexpr optional<T> make_optional(Args&&... args)
{
    return optional<T>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename Policy, typename... Args>
[[nodiscard]] constexpr optional<T, Policy> make_optional(Args&&... args)
{
    return optional<T, Policy>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename U, typename... Args>
[[nodiscard]] constexpr optional<T>
    make_optional(std::initializer_list<U> ilist, Args&&... args)
{
    return optional<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

template <typename T, typename Policy, typename U, typename... Args>
[[nodiscard]] constexpr optional<T, Policy>
    make_optional(std::initializer_list<U> ilist, Args&&... args)
{
    return optional<T, Policy>{std::in_place, ilist, std::forward<Args>(args)...};
}

template <typename T>
class optional_reference;

namespace details {

template <typename T, typename U>
constexpr bool converts_from_optional_reference =
    std::is_constructible_v<T&, const util::optional_reference<U>&> ||
    std::is_constructible_v<T&, util::optional_reference<U>&> ||
    std::is_constructible_v<T&, const util::optional_reference<U>&&> ||
    std::is_constructible_v<T&, util::optional_reference<U>&&> ||
    std::is_convertible_v<const util::optional_reference<U>&, T&> ||
    std::is_convertible_v<util::optional_reference<U>&, T&> ||
    std::is_convertible_v<const util::optional_reference<U>&&, T&> ||
    std::is_convertible_v<util::optional_reference<U>&&, T&>;

} // namespace details

// Type that has identical semantics to a pointer without arithmetics
// and with the additional interface of std::optional.
template <typename T>
class optional_reference
{
public:
    using value_type = T&;

    optional_reference() = default;

    constexpr optional_reference(std::nullopt_t) noexcept {}

    template <typename U = T,
        Q_UTIL_REQUIRES(std::is_constructible_v<T&, U&>)>
    constexpr optional_reference(U& u) noexcept
        : m_ptr{std::addressof(static_cast<T&>(u))} {}

    template <
        typename U,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> && std::is_constructible_v<T&, U&> &&
            std::is_convertible_v<U&, T&> && !details::converts_from_optional_reference<T, U>)>
    optional_reference(const optional_reference<U>& other) noexcept
    {
        *this = other;
    }

    template <typename U,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> && std::is_constructible_v<T&, U&> &&
            !std::is_convertible_v<U&, T&> && !details::converts_from_optional_reference<T, U>)>
    explicit optional_reference(const optional_reference<U>& other) noexcept
    {
        if (other)
            *this = static_cast<T&>(*other);
        else
            *this = std::nullopt;
    }

    constexpr optional_reference& operator=(std::nullopt_t) noexcept
    {
        m_ptr = nullptr;
        return *this;
    }

    template <typename U = T,
        Q_UTIL_REQUIRES(std::is_constructible_v<T&, U&> && std::is_convertible_v<U&, T&>)>
    constexpr optional_reference& operator=(U& u) noexcept
    {
        m_ptr = std::addressof(static_cast<T&>(u));
        return *this;
    }

    template <typename U,
        Q_UTIL_REQUIRES(
            !std::is_same_v<T, U> && std::is_constructible_v<T&, U&> &&
            std::is_convertible_v<U&, T&> && !details::converts_from_optional_reference<T, U>)>
    constexpr optional_reference& operator=(const optional_reference<U>& other) noexcept
    {
        if (other)
            return *this = *other;
        else
            return *this = std::nullopt;
    }

    template <typename U = T>
    optional_reference& emplace(U& u) noexcept
    {
        return *this = u;
    }

    constexpr void reset() noexcept { m_ptr = nullptr; }

    constexpr void swap(optional_reference& other) noexcept
    {
        const auto backup = m_ptr;
        m_ptr = other.m_ptr;
        other.mptr = backup;
    }

    constexpr T* operator->() const noexcept { return m_ptr; }

    constexpr T& operator*() const noexcept { return *m_ptr; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return m_ptr != nullptr; }

    explicit constexpr operator bool() const noexcept { return m_ptr != nullptr; }

    [[nodiscard]] constexpr T& value() const
    {
        if (has_value())
            return *m_ptr;
        throw std::bad_optional_access{};
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& u) const
    {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return has_value() ? **this : static_cast<T>(std::forward<U>(u));
    }

private:
    T* m_ptr = nullptr;
};

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) == static_cast<bool>(rhs) && (!lhs || *lhs == *rhs);
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) != static_cast<bool>(rhs) ||
        (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
constexpr bool operator>(const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs > *rhs);
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs <= *rhs);
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs >= *rhs);
}

// Comparisons with nullopt.

template <typename T>
[[nodiscard]] constexpr bool operator==(const optional_reference<T>& lhs, const std::nullopt_t) noexcept
{
    return !lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(const std::nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return !rhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(const optional_reference<T>& lhs, std::nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(std::nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator<(const optional_reference<T>&, std::nullopt_t) noexcept
{
    return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator<(std::nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator>(const optional_reference<T>& lhs, std::nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator>(std::nullopt_t, const optional_reference<T>&) noexcept
{
    return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator<=(const optional_reference<T>& lhs, std::nullopt_t) noexcept
{
    return !lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator<=(std::nullopt_t, const optional_reference<T>&) noexcept
{
    return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator>=(const optional_reference<T>&, std::nullopt_t) noexcept
{
    return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator>=(std::nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return !rhs;
}

// Comparisons with value type.

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs == rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs == *rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs != rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs != *rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs < rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs < *rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs > rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs > *rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs <= rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs <= *rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs >= rhs;
}

template <typename T, typename U,
    Q_UTIL_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs >= *rhs;
}

template <typename T>
constexpr void swap(optional_reference<T>& lhs, optional_reference<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace q::util
