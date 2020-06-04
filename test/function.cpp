#include <dze/functional.hpp>

#include <array>
#include <cstdarg>
#include <functional>

#include <catch2/catch.hpp>

namespace {

struct callable_but_not_copyable
{
    callable_but_not_copyable() = default;

    callable_but_not_copyable(const callable_but_not_copyable&) = delete;
    callable_but_not_copyable(callable_but_not_copyable&&) = delete;
    callable_but_not_copyable& operator=(const callable_but_not_copyable&) = delete;
    callable_but_not_copyable& operator=(callable_but_not_copyable&&) = delete;

    template <class... Args>
    void operator()(Args&&...) const {}
};

} // namespace

TEST_CASE("Traits")
{
    SECTION("Constructibility")
    {
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<void()>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<void()>, callable_but_not_copyable&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<void() const>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<void() const>, callable_but_not_copyable&>);

        STATIC_REQUIRE(
            std::is_constructible_v<q::util::function<int(int)>, q::util::function<int(int) const>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(int) const>, q::util::function<int(int)>>);
        STATIC_REQUIRE(
            std::is_constructible_v<q::util::function<int(short)>, q::util::function<short(int) const>>);
        STATIC_REQUIRE(
            std::is_constructible_v<q::util::function<short(int)>, q::util::function<int(short) const>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(short) const>, q::util::function<short(int)>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(int)>, q::util::function<int(int) const>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(int) const>, q::util::function<int(int)>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(short)>, q::util::function<short(int) const>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<int(short) const>, q::util::function<short(int)>&>);

        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<q::util::function<int(int)>, q::util::function<int(int) const>>);

        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<const int&()>, int (*)()>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<const int&() const>, int (*)()>);

        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<const int&() noexcept>, int (*)()>);
        STATIC_REQUIRE(
            !std::is_constructible_v<q::util::function<const int&() const noexcept>, int (*)()>);
    }

    SECTION("Assignability")
    {
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<void()>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<void()>, callable_but_not_copyable&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<void() const>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<void() const>, callable_but_not_copyable&>);

        STATIC_REQUIRE(
            std::is_assignable_v<q::util::function<int(int)>, q::util::function<int(int) const>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(int) const>, q::util::function<int(int)>>);
        STATIC_REQUIRE(
            std::is_assignable_v<q::util::function<int(short)>, q::util::function<short(int) const>>);
        STATIC_REQUIRE(
            std::is_assignable_v<q::util::function<short(int)>, q::util::function<int(short) const>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(short) const>, q::util::function<short(int)>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(int)>, q::util::function<int(int) const>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(int) const>, q::util::function<int(int)>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(short)>, q::util::function<short(int) const>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<q::util::function<int(short) const>, q::util::function<short(int)>&>);

        STATIC_REQUIRE(
            std::is_nothrow_assignable_v<q::util::function<int(int)>, q::util::function<int(int) const>>);
    }
}

namespace {

template <class T, size_t S>
struct functor
{
    std::array<T, S> data = {0};

    // Two operator() with different argument types.
    // The InvokeReference tests use both
    T const& operator()(const size_t index) const { return data[index]; }

    T operator()(const size_t index, T const& value)
    {
        T oldvalue = data[index];
        data[index] = value;
        return oldvalue;
    }
};

} // namespace

TEST_CASE("Invoke functor")
{
    functor<int, 100> func;
    func(5, 42);
    q::util::function<int(size_t)> getter = func;

    CHECK(getter(5) == 42);
}

TEST_CASE("Invoke reference")
{
    functor<int, 10> func;
    func(5, 123);

    // Have Functions for getter and setter, both referencing the same funtor
    q::util::function<int(size_t) const> getter = std::ref(func);
    q::util::function<int(size_t, int)> setter = std::ref(func);

    CHECK(getter(5) == 123);
    CHECK(setter(5, 456) == 123);
    CHECK(setter(5, 567) == 456);
    CHECK(getter(5) == 567);
}

namespace {

int func_int_int_add_25(const int x) { return x + 25; }

} // namespace

TEST_CASE("Emptiness")
{
    SECTION("Default constructed")
    {
        q::util::function<int(int)> f;
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }

    SECTION("Initialized with lambda")
    {
        q::util::function f = [] (int x) { return x + 1; };
        CHECK(f != nullptr);
        CHECK(nullptr != f);
        CHECK(f);
        CHECK(f(99) == 100);
    }

    SECTION("Initialized with function pointer")
    {
        q::util::function f = &func_int_int_add_25;
        CHECK(f != nullptr);
        CHECK(nullptr != f);
        CHECK(f);
        CHECK(f(100) == 125);
    }

    SECTION("Initialized with default constructed object")
    {
        q::util::function f{q::util::function<int(int)>{}};
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }

    SECTION("Assigned nullptr")
    {
        q::util::function f = &func_int_int_add_25;
        f = nullptr;
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }
}

namespace {

int func_int_int_add_111(const int x) { return x + 111; }

} // namespace

TEST_CASE("Swap")
{
    q::util::function mf1{func_int_int_add_25};
    q::util::function mf2{func_int_int_add_111};

    mf1.swap(mf2);
    CHECK(mf2(100) == 125);
    CHECK(mf1(100) == 211);

    q::util::function<int(int)> mf3{nullptr};

    mf1.swap(mf3);
    CHECK(mf3(100) == 211);
    CHECK(!mf1);

    q::util::function mf4{[] (int x) mutable { return x + 222; }};

    mf4.swap(mf3);
    CHECK(mf4(100) == 211);
    CHECK(mf3(100) == 322);

    mf3.swap(mf1);
    CHECK(!mf3);
    CHECK(mf1(100) == 322);
}

TEST_CASE("Non-copyable lambda")
{
    auto unique_ptr_int = std::make_unique<int>(900);

    auto functor =
        [up = std::move(unique_ptr_int)] () mutable
        {
            return ++*up;
        };

    CHECK(functor() == 901);

    q::util::function<int(void)> func = std::move(functor);

    CHECK(func() == 902);
}

TEST_CASE("Overloaded functor")
{
    struct overloaded_functor
    {
        int operator()(int x) { return 100 + 1 * x; }

        int operator()(int x) const { return 100 + 2 * x; }

        int operator()(int x, int) { return 100 + 3 * x; }

        int operator()(int x, int) const { return 100 + 4 * x; }

        int operator()(int x, const char*) { return 100 + 5 * x; }

        int operator()(int x, const std::vector<int>&) const { return 100 + 6 * x; }
    };
    overloaded_functor of;

    q::util::function<int(int)> variant1 = of;
    CHECK(variant1(15) == 100 + 1 * 15);

    q::util::function<int(int) const> variant2 = of;
    CHECK(variant2(16) == 100 + 2 * 16);

    q::util::function<int(int, int)> variant3 = of;
    CHECK(variant3(17, 0) == 100 + 3 * 17);

    q::util::function<int(int, int) const> variant4 = of;
    CHECK(variant4(18, 0) == 100 + 4 * 18);

    q::util::function<int(int, const char*)> variant5 = of;
    CHECK(variant5(19, "foo") == 100 + 5 * 19);

    q::util::function<int(int, const std::vector<int>&)> variant6 = of;
    CHECK(variant6(20, {}) == 100 + 6 * 20);
    CHECK(variant6(20, {1, 2, 3}) == 100 + 6 * 20);

    q::util::function<int(int, const std::vector<int>&) const> variant6_const = of;
    CHECK(variant6_const(21, {}) == 100 + 6 * 21);

    q::util::function variant2_nonconst = std::move(variant2);
    CHECK(variant2_nonconst(23) == 100 + 2 * 23);

    q::util::function variant4_nonconst = std::move(variant4);
    CHECK(variant4_nonconst(25, 0) == 100 + 4 * 25);

    q::util::function<int(int, const std::vector<int>&)> variant6_const_nonconst =
        std::move(variant6_const);
    CHECK(variant6_const_nonconst(28, {}) == 100 + 6 * 28);
}

TEST_CASE("Lambda")
{
    q::util::function func = [] (int x) { return 1000 + x; };
    CHECK(func(1) == 1001);

    q::util::function<int(int) const> func_const = [] (int x) { return 2000 + x; };
    CHECK(func_const(1) == 2001);

    int number = 3000;
    q::util::function<int()> func_mutable = [number] () mutable { return ++number; };
    CHECK(func_mutable() == 3001);
    CHECK(func_mutable() == 3002);

    q::util::function<int(int)> func_const_made_nonconst = std::move(func_const);
    CHECK(func_const_made_nonconst(2) == 2002);
}

TEST_CASE("Members")
{
    struct member_func
    {
        int x;

        [[nodiscard]] int get() const { return x; }

        void set(const int xx) { x = xx; }
    };

    SECTION("Data")
    {
        member_func mf;
        member_func const& cmf = mf;
        mf.x = 123;

        q::util::function<int(member_func const*)> data_getter1 = &member_func::x;
        CHECK(data_getter1(&cmf) == 123);
        q::util::function<int(member_func*)> data_getter2 = &member_func::x;
        CHECK(data_getter2(&mf) == 123);
        q::util::function<int(member_func const&)> data_getter3 = &member_func::x;
        CHECK(data_getter3(cmf) == 123);
        q::util::function<int(member_func&)> data_getter4 = &member_func::x;
        CHECK(data_getter4(mf) == 123);
    }

    SECTION("Method")
    {
        member_func mf;
        member_func const& cmf = mf;
        mf.x = 123;

        q::util::function<int(member_func const*)> getter1 = &member_func::get;

        CHECK(getter1(&cmf) == 123);
        q::util::function<int(member_func*)> getter2 = &member_func::get;
        CHECK(getter2(&mf) == 123);
        q::util::function<int(member_func const&)> getter3 = &member_func::get;
        CHECK(getter3(cmf) == 123);
        q::util::function<int(member_func&)> getter4 = &member_func::get;
        CHECK(getter4(mf) == 123);

        q::util::function<void(member_func*, int)> setter1 = &member_func::set;
        setter1(&mf, 234);
        CHECK(mf.x == 234);

        q::util::function<void(member_func&, int)> setter2 = &member_func::set;
        setter2(mf, 345);
        CHECK(mf.x == 345);
    }
}

TEST_CASE("Copy and move")
{
    struct constructor_tag_t {};

    constexpr constructor_tag_t constructor_tag;

    class copy_move_tracker
    {
    public:
        copy_move_tracker() = delete;

        explicit copy_move_tracker(constructor_tag_t)
            : m_data(std::make_shared<std::pair<size_t, size_t>>(0, 0)) {}

        copy_move_tracker(const copy_move_tracker& other) noexcept
            : m_data(other.m_data)
        {
            ++m_data->first;
        }

        copy_move_tracker& operator=(const copy_move_tracker& other) noexcept
        {
            m_data = other.m_data;
            ++m_data->first;
            return *this;
        }

        copy_move_tracker(copy_move_tracker&& other) noexcept
            : m_data(other.m_data) // NOLINT(performance-move-constructor-init)
        {
            ++m_data->second;
        }

        copy_move_tracker& operator=(copy_move_tracker&& other) noexcept
        {
            m_data = other.m_data;
            ++m_data->second;
            return *this;
        }

        [[nodiscard]] size_t copy_count() const { return m_data->first; }

        [[nodiscard]] size_t move_count() const { return m_data->second; }

        void reset_counters() { m_data->first = m_data->second = 0; }

    private:
        std::shared_ptr<std::pair<size_t, size_t>> m_data;
    };

    SECTION("Capture")
    {
        copy_move_tracker cmt{constructor_tag};
        CHECK(cmt.copy_count() == 0);
        CHECK(cmt.move_count() == 0);

        SECTION("Copy")
        {
            auto lambda = [cmt = std::move(cmt)] { return cmt.move_count(); };
            q::util::function f = lambda;

            CHECK(cmt.move_count() + cmt.copy_count() <= 4);
            CHECK(cmt.copy_count() <= 1);

            cmt.reset_counters();
            f();
            CHECK(cmt.copy_count() == 0);
            CHECK(cmt.move_count() == 0);
        }

        SECTION("Move")
        {
            auto lambda = [cmt = std::move(cmt)] { return cmt.move_count(); };
            q::util::function f = std::move(lambda);

            CHECK(cmt.move_count() <= 4);
            CHECK(cmt.copy_count() == 0);

            cmt.reset_counters();
            f();
            CHECK(cmt.copy_count() == 0);
            CHECK(cmt.move_count() == 0);
        }
    }

    SECTION("Parameter")
    {
        copy_move_tracker cmt{constructor_tag};
        CHECK(cmt.copy_count() == 0);
        CHECK(cmt.move_count() == 0);

        SECTION("Copy")
        {
            // NOLINTNEXTLINE(performance-unnecessary-value-param)
            q::util::function f = [] (copy_move_tracker c) { return c.move_count(); };

            cmt.reset_counters();
            f(cmt);
            CHECK(cmt.move_count() + cmt.copy_count() <= 4);
            CHECK(cmt.copy_count() <= 1);

            cmt.reset_counters();
            f(std::move(cmt));
            CHECK(cmt.move_count() == 4);
            CHECK(cmt.copy_count() == 0);
        }

        SECTION("Mutable reference")
        {
            q::util::function f = [] (copy_move_tracker& c) { return c.move_count(); };

            cmt.reset_counters();
            f(cmt);
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }

        SECTION("Const reference")
        {
            q::util::function f = [] (const copy_move_tracker& c) { return c.move_count(); };

            cmt.reset_counters();
            f(cmt);
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }

        SECTION("R-value reference")
        {
            q::util::function f = [] (copy_move_tracker&& c) { return c.move_count(); };

            cmt.reset_counters();
            f(std::move(cmt));
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }
    }
}

namespace {

// Templates cannot be declared inside of a local class.
struct variadic_template_sum
{
    template <typename... Args>
    int operator()(Args... args) const { return (args + ...); }
};

} // namespace

TEST_CASE("Variadic template")
{
    q::util::function<int(int)> f1 = variadic_template_sum();
    q::util::function<int(int, int)> f2 = variadic_template_sum();
    q::util::function<int(int, int, int)> f3 = variadic_template_sum();

    CHECK(f1(66) == 66);
    CHECK(f2(55, 44) == 99);
    CHECK(f3(33, 22, 11) == 66);
}

TEST_CASE("Variadic arguments")
{
    struct variadic_arguments_sum
    {
        int operator()(int count, ...) const
        {
            int result = 0;
            va_list args;
            va_start(args, count);
            for (int i = 0; i < count; ++i)
                result += va_arg(args, int);

            va_end(args);
            return result;
        }
    };

    q::util::function<int(int)> f1 = variadic_arguments_sum();
    q::util::function<int(int, int)> f2 = variadic_arguments_sum();
    q::util::function<int(int, int, int)> f3 = variadic_arguments_sum();

    CHECK(f1(0) == 0);
    CHECK(f2(1, 66) == 66);
    CHECK(f3(2, 55, 44) == 99);
}

namespace {

template <typename T>
void for_each(
    const T& range, const q::util::function<void(const typename T::value_type&) const>& func)
{
    for (auto& elem : range)
        func(elem);
}

} // namespace

TEST_CASE("Safe capture by reference")
{
    // A function can use const q::util::function& as a parameter to signal that
    // it is safe to pass a lambda that captures local variables by reference.
    // It is safe because we know the function called can only invoke the
    // q::util::function until it returns. It can't store a copy of the
    // q::util::function (because it's not copyable), and it can't move the
    // q::util::function somewhere else (because it gets only a const&).

    const std::vector vec = {20, 30, 40, 2, 3, 4, 200, 300, 400};

    int sum = 0;

    // for_each's second parameter is of type const q::util::function<...>&.
    // Hence we know we can safely pass it a lambda that references local
    // variables. There is no way the reference to x will be stored anywhere.
    for_each(vec, [&sum] (const int x) { sum += x; });

    CHECK(sum == 999);
}

TEST_CASE("Ignore return value")
{
    int x = 95;

    // Assign a lambda that return int to a dze::function that returns void.
    q::util::function<void()> f = [&] () { return ++x; };

    CHECK(x == 95);
    f();
    CHECK(x == 96);

    q::util::function g = [&] () { return ++x; };
    q::util::function<void()> cg = std::move(g);

    CHECK(x == 96);
    cg();
    CHECK(x == 97);
}

TEST_CASE("Convertibility")
{
    struct c_base
    {
        int x;
    };

    struct c_derived : c_base {};

    SECTION("From non-function")
    {
        q::util::function<double()> f1 = [] () { return 5; };
        CHECK(f1() == 5.0);

        q::util::function<int()> f2 = [] () { return 5.2; };
        CHECK(f2() == 5);

        c_derived derived;
        derived.x = 55;

        q::util::function<const c_base&()> f3 = [&] () -> const c_derived& { return derived; };
        CHECK(f3().x == 55);

        q::util::function<const c_base&()> f4 = [&] () -> c_derived& { return derived; };
        CHECK(f4().x == 55);

        q::util::function<c_base&()> f5 = [&] () -> c_derived& { return derived; };
        CHECK(f5().x == 55);

        q::util::function<const c_base*()> f6 = [&] () -> const c_derived* { return &derived; };
        CHECK(55 == f6()->x);

        q::util::function<const c_base*()> f7 = [&] () -> c_derived* { return &derived; };
        CHECK(f7()->x == 55);

        q::util::function<c_base*()> f8 = [&] () -> c_derived* { return &derived; };
        CHECK(f8()->x == 55);

        q::util::function<c_base()> f9 = [&] () -> c_derived
        {
            auto d = derived;
            d.x = 66;
            return d;
        };
        CHECK(f9().x == 66);
    }

    SECTION("From function r-value")
    {
        q::util::function<int()> f1 = [] () { return 5; };
        q::util::function<double()> cf1 = std::move(f1);
        CHECK(cf1() == 5.0);
        q::util::function<int()> ccf1 = std::move(cf1);
        CHECK(ccf1() == 5);

        q::util::function<double()> f2 = [] () { return 5.2; };
        q::util::function<int()> cf2 = std::move(f2);
        CHECK(cf2() == 5);
        q::util::function<double()> ccf2 = std::move(cf2);
        CHECK(ccf2() == 5.0);

        c_derived derived;
        derived.x = 55;

        q::util::function f3 = [&] () -> const c_derived& { return derived; };
        q::util::function<const c_base&()> cf3 = std::move(f3);
        CHECK(cf3().x == 55);

        q::util::function f4 = [&] () -> c_derived& { return derived; };
        q::util::function<const c_base&()> cf4 = std::move(f4);
        CHECK(cf4().x == 55);

        q::util::function f5 = [&] () -> c_derived& { return derived; };
        q::util::function<c_base&()> cf5 = std::move(f5);
        CHECK(cf5().x == 55);

        q::util::function f6 = [&] () -> const c_derived* { return &derived; };
        q::util::function<const c_base*()> cf6 = std::move(f6);
        CHECK(cf6()->x == 55);

        q::util::function<const c_derived*()> f7 = [&] () -> c_derived* { return &derived; };
        q::util::function<const c_base*()> cf7 = std::move(f7);
        CHECK(cf7()->x == 55);

        q::util::function f8 = [&] () -> c_derived* { return &derived; };
        q::util::function<c_base*()> cf8 = std::move(f8);
        CHECK(cf8()->x == 55);

        q::util::function f9 = [&] () -> c_derived {
            auto d = derived;
            d.x = 66;
            return d;
        };
        q::util::function<c_base()> cf9 = std::move(f9);
        CHECK(cf9().x == 66);
    }
}

namespace {

template <typename return_t, typename... Args>
void deduce_args(q::util::function<return_t(Args...) const>) {}

} // namespace

TEST_CASE("Deducable arguments")
{
    deduce_args(q::util::function{[] {}});
    deduce_args(q::util::function{[] (int, float) {}});
    deduce_args(q::util::function{[] (int i, float) { return i; }});
}

TEST_CASE("Constructor with copy")
{
    struct X
    {
        X() = default;
        X(const X&) noexcept = default;
        X& operator=(const X&) = default;
    };

    struct Y
    {
        Y() = default;
        Y(const Y&) noexcept(false) {} // NOLINT(modernize-use-equals-default)
        Y(Y&&) noexcept = default;
        Y& operator=(const Y&) = default;
        Y& operator=(Y&&) = default;
    };

    auto lx = [x = X{}] { (void)x; };
    auto ly = [y = Y{}] { (void)y; };

    STATIC_REQUIRE(noexcept(q::util::function{lx}));
    STATIC_REQUIRE(!noexcept(q::util::function{ly}));
}