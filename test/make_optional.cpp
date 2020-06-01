#include "optional.hpp"

#include <tuple>
#include <vector>

#include <catch2/catch.hpp>

struct takes_init_and_variadic
{
    std::vector<int> v;
    std::tuple<int, int> t;

    template <typename... args_t>
    takes_init_and_variadic(std::initializer_list<int> l, args_t&&... args)
        : v{l}
        , t{std::forward<args_t>(args)...} {}
};

namespace qut = q::util::test;

TEST_CASE("Make optional", "[make_optional]")
{
    constexpr auto o1 = qut::make_optional(42);
    STATIC_REQUIRE(std::is_same_v<decltype(o1), const qut::optional<int>>);
    STATIC_REQUIRE(o1.has_value());
    STATIC_REQUIRE(o1 == qut::optional<int>{42});

    constexpr auto o2 = qut::make_optional<std::tuple<int, int, int, int>>(0, 1, 2, 3);
    STATIC_REQUIRE(o2.has_value());
    STATIC_REQUIRE(std::get<0>(*o2) == 0);
    STATIC_REQUIRE(std::get<1>(*o2) == 1);
    STATIC_REQUIRE(std::get<2>(*o2) == 2);
    STATIC_REQUIRE(std::get<3>(*o2) == 3);

    auto o3 = qut::make_optional<std::vector<int>>({0, 1, 2, 3});
    REQUIRE(o3.has_value());
    CHECK((*o3)[0] == 0);
    CHECK((*o3)[1] == 1);
    CHECK((*o3)[2] == 2);
    CHECK((*o3)[3] == 3);

    auto o4 = qut::make_optional<takes_init_and_variadic>({0, 1}, 2, 3);
    REQUIRE(o4.has_value());
    CHECK(o4->v[0] == 0);
    CHECK(o4->v[1] == 1);
    CHECK(std::get<0>(o4->t) == 2);
    CHECK(std::get<1>(o4->t) == 3);
}
