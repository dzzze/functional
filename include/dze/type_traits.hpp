#pragma once

#include <type_traits>

#include <concepts/type_traits.hpp>

// Could be used in template prototypes to match the signature without the default value.
#define Q_UTIL_REQUIRES_PROTO(...) std::enable_if_t<__VA_ARGS__, int>

// Variadic macro needed because of potential commas in the macro argument.
#define Q_UTIL_REQUIRES(...) Q_UTIL_REQUIRES_PROTO(__VA_ARGS__) = 0

namespace q::util {

using namespace concepts;

template <typename enum_t>
constexpr auto underlying_value(const enum_t e) noexcept
{
    return static_cast<std::underlying_type_t<enum_t>>(e);
}

// True if and only if T can be implicitly constructible to U.
template <typename T, typename U>
struct is_implicitly_constructible
    : std::conjunction<std::is_constructible<T, U>, std::is_convertible<U, T>> {};

template <typename T, typename U>
inline constexpr bool is_implicitly_constructible_v = is_implicitly_constructible<T, U>::value;

// True if and only if T can be explicitly constructible to U.
template <typename T, typename U>
struct is_explicitly_constructible : std::conjunction<
    std::is_constructible<T, U>, std::negation<std::is_convertible<U, T>>> {};

template <typename T, typename U>
inline constexpr bool is_explicitly_constructible_v = is_explicitly_constructible<T, U>::value;

// True if and only if T can be explicitly convertible to U.
template <typename T, typename U>
struct is_explicitly_convertible : is_explicitly_constructible<U, T> {};

template <typename T, typename U>
inline constexpr bool is_explicitly_convertible_v = is_explicitly_convertible<T, U>::value;

template <typename T>
struct is_movable : std::conjunction<
    std::is_object<T>,
    std::is_move_constructible<T>,
    std::is_assignable<T&, T>,
    std::is_swappable<T>> {};

template <typename T>
inline constexpr bool is_movable_v = is_movable<T>::value;

template <typename T>
struct is_copyable : std::conjunction<
    std::is_copy_constructible<T>, is_movable<T>, std::is_assignable<T&, const T&>> {};

template <typename T>
inline constexpr bool is_copyable_v = is_copyable<T>::value;

template <typename T>
struct is_referenceable : std::negation<std::is_void<T>> {};

template <typename T>
inline constexpr bool is_referenceable_v = is_referenceable<T>::value;

template <typename T, typename = void>
struct is_defined : std::false_type {};

template <typename T>
struct is_defined<T, std::enable_if_t<(sizeof(T) > 0)>> : std::true_type {};

template <typename T>
inline constexpr bool is_defined_v = is_defined<T>::value;

// https://en.cppreference.com/w/cpp/experimental/is_detected

struct nonesuch
{
    ~nonesuch() = delete;
    nonesuch(const nonesuch&) = delete;
    void operator=(const nonesuch&) = delete;
};

namespace details {

template <
    typename default_t,
    typename always_void_t,
    template <typename...> typename trait,
    typename... args>
struct detector
{
    using value_t = std::false_type;
    using type = default_t;
};

template <typename default_t, template <typename...> typename trait, typename... args>
struct detector<default_t, std::void_t<trait<args...>>, trait, args...>
{
    using value_t = std::true_type;
    using type = trait<args...>;
};

} // namespace details

template <template <typename...> typename trait, typename... args>
using is_detected = typename details::detector<nonesuch, void, trait, args...>::value_t;

template <template <typename...> typename trait, typename... args>
inline constexpr bool is_detected_v = is_detected<trait, args...>::value;

template <template <typename...> typename trait, typename... args>
using detected_t = typename details::detector<nonesuch, void, trait, args...>::type;

template <typename expected, template <typename...> typename trait, typename... args>
using is_detected_exact = std::is_same<expected, detected_t<trait, args...>>;

template <typename expected, template <typename...> typename trait, typename... args>
inline constexpr bool is_detected_exact_v =
    is_detected_exact<expected, trait, args...>::value;

template <typename T>
using incrementability = decltype(++std::declval<T&>());

template <typename T>
struct is_incrementable : is_detected_exact<T&, incrementability, T> {};

template <typename T>
inline constexpr bool is_incrementable_v = is_incrementable<T>::value;

template <typename T>
using dereferenceability = decltype(*std::declval<T&>());

template <typename T>
struct is_dereferenceable : std::conjunction<
    is_detected<dereferenceability, T>, is_referenceable<detected_t<dereferenceability, T>>> {};

template <typename T>
inline constexpr bool is_dereferenceable_v = is_dereferenceable<T>::value;

template <typename T, typename U>
using equality_comparability = decltype(
    std::declval<const std::remove_reference_t<T>&>() ==
    std::declval<const std::remove_reference_t<U>&>());

template <typename T>
struct is_equality_comparable : std::conjunction<
    is_detected<equality_comparability, T, T>,
    std::is_same<detected_t<equality_comparability, T, T>, bool>> {};

} // namespace q::util
