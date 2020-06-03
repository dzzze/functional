#include <dze/optional_reference.hpp>

#include <catch2/catch.hpp>

namespace {

constexpr int i = 42;

class derived;

class base
{
public:
    explicit constexpr base(const int val) noexcept
        : m_val{val} {}

    constexpr operator int() const noexcept { return m_val; }

    explicit constexpr operator const derived&() const noexcept;

    explicit constexpr operator derived&() noexcept;

private:
    int m_val;
};

class derived : public base
{
    using base::base;
};

constexpr base::operator const derived&() const noexcept
{
    return *static_cast<const derived*>(this);
}

constexpr base::operator derived&() noexcept { return *static_cast<derived*>(this); }

constexpr base b{42};
constexpr derived d{84};

} // namespace

TEST_CASE("Constructors", "[constructors]")
{
    SECTION("empty construct")
    {
        using test_t = dze::optional_reference<const int>;

        constexpr test_t o1;
        constexpr test_t o2{};
        constexpr test_t o3 = {};
        constexpr test_t o4 = std::nullopt;
        constexpr test_t o5 = {std::nullopt};
        constexpr test_t o6(std::nullopt);

        STATIC_REQUIRE(!o1);
        STATIC_REQUIRE(!o2);
        STATIC_REQUIRE(!o3);
        STATIC_REQUIRE(!o4);
        STATIC_REQUIRE(!o5);
        STATIC_REQUIRE(!o6);
    }

    SECTION("value construct")
    {
        using test_t = dze::optional_reference<const int>;

        constexpr test_t o1 = i;
        constexpr test_t o2{i};
        constexpr test_t o3(i);
        constexpr test_t o4 = {i};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
    }

    SECTION("construct from another")
    {
        using test_t = dze::optional_reference<const int>;

        constexpr test_t o1 = i;
        constexpr test_t o2 = o1;
        constexpr test_t o3{o1};
        constexpr test_t o4(o1);
        constexpr test_t o5 = {o1};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
        STATIC_REQUIRE(*o5 == 42);
    }

    SECTION("implicit construct from another optional type")
    {
        using test_t = dze::optional_reference<const base>;

        dze::optional_reference<const derived> o1 = d;
        test_t o2 = o1;
        test_t o3{o1};
        test_t o4(o1);
        test_t o5 = {o1};

        REQUIRE(*o1 == 84);
        REQUIRE(*o2 == 84);
        REQUIRE(*o3 == 84);
        REQUIRE(*o4 == 84);
        REQUIRE(*o5 == 84);
    }

    SECTION("explicit construct from another optional type")
    {
        using test_t = dze::optional_reference<const derived>;

        dze::optional_reference<const base> o1 = b;
        test_t o2{o1};
        test_t o3(o1);

        REQUIRE(*o1 == 42);
        REQUIRE(*o2 == 42);
        REQUIRE(*o3 == 42);
    }
}

TEST_CASE("Assignment", "[assignment]")
{
    SECTION("Assign value")
    {
        using test_t = dze::optional_reference<const int>;

        test_t o1 = i;
        int j = 84;

        o1 = j;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Assign value of different type")
    {
        using test_t = dze::optional_reference<const base>;

        test_t o1 = b;
        o1 = d;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Self assign")
    {
        using test_t = dze::optional_reference<const int>;

        test_t o1 = i;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == 42);
    }

    SECTION("Assign another optional")
    {
        using test_t = dze::optional_reference<const int>;

        int j = 84;
        test_t o1 = i;
        test_t o2 = j;

        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional")
    {
        using test_t = dze::optional_reference<const int>;

        test_t o1 = i;
        test_t o2;

        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        using test_t = dze::optional_reference<const int>;

        test_t o1 = i;

        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        using test_t = dze::optional_reference<const base>;

        test_t o1 = b;
        dze::optional_reference<const derived> o2 = d;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional of different type")
    {
        using test_t = dze::optional_reference<const base>;

        test_t o1 = b;
        dze::optional_reference<const derived> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}

TEST_CASE("Emplace", "[emplace]")
{
    SECTION("Emplace value")
    {
        using test_t = dze::optional_reference<const int>;

        test_t o1 = i;
        int j = 84;

        o1.emplace(j);
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Emplace value of different type")
    {
        using test_t = dze::optional_reference<const base>;

        test_t o1 = b;
        o1.emplace(d);
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }
}
