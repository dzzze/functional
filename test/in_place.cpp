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

TEST_CASE("In place", "[in_place]")
{
    dze::optional<int> o1{std::in_place};
    dze::optional<int> o2(std::in_place);
    REQUIRE(o1);
    REQUIRE(o1 == 0);
    REQUIRE(o2);
    REQUIRE(o2 == 0);

    dze::optional<int> o3(std::in_place, 42);
    REQUIRE(o3 == 42);

    dze::optional<std::tuple<int, int>> o4(std::in_place, 0, 1);
    REQUIRE(o4);
    REQUIRE(std::get<0>(*o4) == 0);
    REQUIRE(std::get<1>(*o4) == 1);

    dze::optional<std::vector<int>> o5(std::in_place, {0, 1});
    REQUIRE(o5);
    REQUIRE((*o5)[0] == 0);
    REQUIRE((*o5)[1] == 1);

    dze::optional<takes_init_and_variadic> o6(std::in_place, {0, 1}, 2, 3);
    REQUIRE(o6->v[0] == 0);
    REQUIRE(o6->v[1] == 1);
    REQUIRE(std::get<0>(o6->t) == 2);
    REQUIRE(std::get<1>(o6->t) == 3);
}
