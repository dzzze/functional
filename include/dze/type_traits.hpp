#pragma once

#include <type_traits>

// Could be used in template prototypes to match the signature without the default value.
#define Q_UTIL_REQUIRES_PROTO(...) std::enable_if_t<__VA_ARGS__, int>

// Variadic macro needed because of potential commas in the macro argument.
#define Q_UTIL_REQUIRES(...) Q_UTIL_REQUIRES_PROTO(__VA_ARGS__) = 0

namespace q::util {

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

} // namespace q::util
