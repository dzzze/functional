#include <dze/functional.hpp>

#include <array>
#include <cstdarg>
#include <functional>

#include <catch2/catch.hpp>

struct callable_but_not_copyable
{
    callable_but_not_copyable() = default;

    callable_but_not_copyable(const callable_but_not_copyable&) = delete;
    callable_but_not_copyable(callable_but_not_copyable&&) = delete;
    callable_but_not_copyable& operator=(const callable_but_not_copyable&) = delete;
    callable_but_not_copyable& operator=(callable_but_not_copyable&&) = delete;

    template <typename... Args>
    void operator()(Args&&...) const {}
};

TEST_CASE("Traits")
{
    SECTION("Constructibility")
    {
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<void()>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<void()>, callable_but_not_copyable&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<void() const>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<void() const>, callable_but_not_copyable&>);

        STATIC_REQUIRE(
            std::is_constructible_v<dze::function<int(int)>, dze::function<int(int) const>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(int) const>, dze::function<int(int)>>);
        STATIC_REQUIRE(
            std::is_constructible_v<dze::function<int(short)>, dze::function<short(int) const>>);
        STATIC_REQUIRE(
            std::is_constructible_v<dze::function<short(int)>, dze::function<int(short) const>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(short) const>, dze::function<short(int)>>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(int)>, dze::function<int(int) const>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(int) const>, dze::function<int(int)>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(short)>, dze::function<short(int) const>&>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<int(short) const>, dze::function<short(int)>&>);

        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<dze::function<int(int)>, dze::function<int(int) const>>);

        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<const int&()>, int (*)()>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<const int&() const>, int (*)()>);

        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<const int&() noexcept>, int (*)()>);
        STATIC_REQUIRE(
            !std::is_constructible_v<dze::function<const int&() const noexcept>, int (*)()>);
    }

    SECTION("Assignability")
    {
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<void()>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<void()>, callable_but_not_copyable&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<void() const>, callable_but_not_copyable>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<void() const>, callable_but_not_copyable&>);

        STATIC_REQUIRE(
            std::is_assignable_v<dze::function<int(int)>, dze::function<int(int) const>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(int) const>, dze::function<int(int)>>);
        STATIC_REQUIRE(
            std::is_assignable_v<dze::function<int(short)>, dze::function<short(int) const>>);
        STATIC_REQUIRE(
            std::is_assignable_v<dze::function<short(int)>, dze::function<int(short) const>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(short) const>, dze::function<short(int)>>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(int)>, dze::function<int(int) const>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(int) const>, dze::function<int(int)>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(short)>, dze::function<short(int) const>&>);
        STATIC_REQUIRE(
            !std::is_assignable_v<dze::function<int(short) const>, dze::function<short(int)>&>);

        STATIC_REQUIRE(
            std::is_nothrow_assignable_v<dze::function<int(int)>, dze::function<int(int) const>>);
    }
}

template <typename T, size_t Size>
struct functor
{
    std::array<T, Size> data = {0};

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

template <template <typename...> typename Function>
void test_invoke_functor()
{
    functor<int, 100> func;
    func(5, 42);
    Function<int(size_t)> getter = func;

    CHECK(getter(5) == 42);
}

TEST_CASE("Invoke functor", "[invoke.functor]")
{
    test_invoke_functor<dze::function>();
    test_invoke_functor<dze::pmr::function>();
}

template <template <typename...> typename Function>
void test_invoke_ref()
{
    functor<int, 10> func;
    func(5, 123);

    // Have Functions for getter and setter, both referencing the same funtor
    Function<int(size_t) const> getter = std::ref(func);
    Function<int(size_t, int)> setter = std::ref(func);

    CHECK(getter(5) == 123);
    CHECK(setter(5, 456) == 123);
    CHECK(setter(5, 567) == 456);
    CHECK(getter(5) == 567);
}

TEST_CASE("Invoke reference", "[invoke.functor]")
{
    test_invoke_ref<dze::function>();
    test_invoke_ref<dze::pmr::function>();
}

TEST_CASE("Emptiness")
{
    SECTION("Default constructed")
    {
        dze::function<int(int)> f;
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }

    SECTION("Initialized with lambda")
    {
        dze::function f = [] (int x) { return x + 1; };
        CHECK(f != nullptr);
        CHECK(nullptr != f);
        CHECK(f);
        CHECK(f(99) == 100);
    }

    SECTION("Initialized with function pointer")
    {
        int(*func_int_int_add_25 )(int) = [] (const int x ) { return x + 25; };
        dze::function f = func_int_int_add_25;
        CHECK(f != nullptr);
        CHECK(nullptr != f);
        CHECK(f);
        CHECK(f(100) == 125);
    }

    SECTION("Initialized with default constructed object")
    {
        dze::function f = dze::function<int(int)>{};
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }

    SECTION("Assigned nullptr")
    {
        dze::function f = [] (int x) { return x + 1; };
        f = nullptr;
        CHECK(f == nullptr);
        CHECK(nullptr == f);
        CHECK(!f);
    }
}

template <template <typename...> typename Function>
void test_swap_nullptr()
{
    Function<int(int)> f1 = [] (const int i) { return i + 25; };
    decltype(f1) f2{nullptr};

    f1.swap(f2);
    CHECK(!f1);
    CHECK(f2(100) == 125);

    f1.swap(f2);
    CHECK(f1(100) == 125);
    CHECK(!f2);
}

template <template <typename...> typename Function>
void test_swap_inline()
{
    Function<int(int)> f1 = [] (const int i) { return i + 25; };
    Function<int(int)> f2 = [] (const int i) { return i + 111; };

    f1.swap(f2);
    CHECK(f1(100) == 211);
    CHECK(f2(100) == 125);

    f1.swap(f2);
    CHECK(f1(100) == 125);
    CHECK(f2(100) == 211);
}

template <template <typename...> typename Function>
void test_swap_allocated_and_inline()
{
    std::array<int, 101> a;
    for (int i = 0; i != static_cast<int>(a.size()); ++i)
        a[i] = i + 111;

    Function<int(int)> f1 = [] (const int i) { return i + 25; };
    Function<int(int)> f2 = [a] (const int i) { return a[i]; };

    f1.swap(f2);
    CHECK(f1(100) == 211);
    CHECK(f2(100) == 125);

    f1.swap(f2);
    CHECK(f1(100) == 125);
    CHECK(f2(100) == 211);
}

template <template <typename...> typename Function>
void test_swap_allocated()
{
    std::array<int, 101> a;
    for (int i = 0; i != static_cast<int>(a.size()); ++i)
        a[i] = i;

    Function<int(int)> f1 = [a] (const int i) { return a[i] + 25; };
    Function<int(int)> f2 = [a] (const int i) { return a[i] + 111; };

    f1.swap(f2);
    CHECK(f1(100) == 211);
    CHECK(f2(100) == 125);

    f1.swap(f2);
    CHECK(f1(100) == 125);
    CHECK(f2(100) == 211);
}

TEST_CASE("Swap")
{
    SECTION("nullptr")
    {
        test_swap_nullptr<dze::function>();
        test_swap_nullptr<dze::pmr::function>();
    }

    SECTION("Both inline")
    {
        test_swap_inline<dze::function>();
        test_swap_inline<dze::pmr::function>();
    }

    SECTION("Allocated & inline")
    {
        test_swap_allocated_and_inline<dze::function>();
        test_swap_allocated_and_inline<dze::pmr::function>();
    }

    SECTION("Both allocated")
    {
        test_swap_allocated<dze::function>();
        test_swap_allocated<dze::pmr::function>();
    }
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

    dze::function func = std::move(functor);

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

    dze::function<int(int)> variant1 = of;
    CHECK(variant1(15) == 100 + 1 * 15);

    dze::function<int(int) const> variant2 = of;
    CHECK(variant2(16) == 100 + 2 * 16);

    dze::function<int(int, int)> variant3 = of;
    CHECK(variant3(17, 0) == 100 + 3 * 17);

    dze::function<int(int, int) const> variant4 = of;
    CHECK(variant4(18, 0) == 100 + 4 * 18);

    dze::function<int(int, const char*)> variant5 = of;
    CHECK(variant5(19, "foo") == 100 + 5 * 19);

    dze::function<int(int, const std::vector<int>&)> variant6 = of;
    CHECK(variant6(20, {}) == 100 + 6 * 20);
    CHECK(variant6(20, {1, 2, 3}) == 100 + 6 * 20);

    dze::function<int(int, const std::vector<int>&) const> variant6_const = of;
    CHECK(variant6_const(21, {}) == 100 + 6 * 21);

    dze::function variant2_nonconst = std::move(variant2);
    CHECK(variant2_nonconst(23) == 100 + 2 * 23);

    dze::function variant4_nonconst = std::move(variant4);
    CHECK(variant4_nonconst(25, 0) == 100 + 4 * 25);

    dze::function<int(int, const std::vector<int>&)> variant6_const_nonconst =
        std::move(variant6_const);
    CHECK(variant6_const_nonconst(28, {}) == 100 + 6 * 28);
}

TEST_CASE("Lambda")
{
    dze::function func_const = [] (const int x) { return 2000 + x; };
    CHECK(func_const(1) == 2001);

    dze::function<int(int)> func_const_to_mut = std::move(func_const);
    CHECK(func_const_to_mut(2) == 2002);

    int number = 3000;
    dze::function func_mut = [number] () mutable { return ++number; };
    CHECK(func_mut() == 3001);
    CHECK(func_mut() == 3002);
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

        dze::function<int(member_func const*)> data_getter1 = &member_func::x;
        CHECK(data_getter1(&cmf) == 123);
        dze::function<int(member_func*)> data_getter2 = &member_func::x;
        CHECK(data_getter2(&mf) == 123);
        dze::function<int(member_func const&)> data_getter3 = &member_func::x;
        CHECK(data_getter3(cmf) == 123);
        dze::function<int(member_func&)> data_getter4 = &member_func::x;
        CHECK(data_getter4(mf) == 123);
    }

    SECTION("Method")
    {
        member_func mf;
        member_func const& cmf = mf;
        mf.x = 123;

        dze::function<int(member_func const*)> getter1 = &member_func::get;

        CHECK(getter1(&cmf) == 123);
        dze::function<int(member_func*)> getter2 = &member_func::get;
        CHECK(getter2(&mf) == 123);
        dze::function<int(member_func const&)> getter3 = &member_func::get;
        CHECK(getter3(cmf) == 123);
        dze::function<int(member_func&)> getter4 = &member_func::get;
        CHECK(getter4(mf) == 123);

        dze::function<void(member_func*, int)> setter1 = &member_func::set;
        setter1(&mf, 234);
        CHECK(mf.x == 234);

        dze::function<void(member_func&, int)> setter2 = &member_func::set;
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
            dze::function f = lambda;

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
            dze::function f = std::move(lambda);

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
            dze::function f = [] (copy_move_tracker c) { return c.move_count(); };

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
            dze::function f = [] (copy_move_tracker& c) { return c.move_count(); };

            cmt.reset_counters();
            f(cmt);
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }

        SECTION("Const reference")
        {
            dze::function f = [] (const copy_move_tracker& c) { return c.move_count(); };

            cmt.reset_counters();
            f(cmt);
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }

        SECTION("R-value reference")
        {
            dze::function f = [] (copy_move_tracker&& c) { return c.move_count(); };

            cmt.reset_counters();
            f(std::move(cmt));
            CHECK(cmt.move_count() == 0);
            CHECK(cmt.copy_count() == 0);
        }
    }
}

struct variadic_template_sum
{
    template <typename... Args>
    int operator()(Args... args) const { return (args + ...); }
};

TEST_CASE("Variadic template")
{
    dze::function<int(int)> f1 = variadic_template_sum();
    dze::function<int(int, int)> f2 = variadic_template_sum();
    dze::function<int(int, int, int)> f3 = variadic_template_sum();

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

    dze::function<int(int)> f1 = variadic_arguments_sum();
    dze::function<int(int, int)> f2 = variadic_arguments_sum();
    dze::function<int(int, int, int)> f3 = variadic_arguments_sum();

    CHECK(f1(0) == 0);
    CHECK(f2(1, 66) == 66);
    CHECK(f3(2, 55, 44) == 99);
}

template <typename T>
void for_each(
    const T& range, const dze::function<void(const typename T::value_type&) const>& func)
{
    for (auto& elem : range)
        func(elem);
}

TEST_CASE("Safe capture by reference")
{
    // A function can use const dze::function& as a parameter to signal that
    // it is safe to pass a lambda that captures local variables by reference.
    // It is safe because we know the function called can only invoke the
    // dze::function until it returns. It can't store a copy of the
    // dze::function (because it's not copyable), and it can't move the
    // dze::function somewhere else (because it gets only a const&).

    const std::vector vec = {20, 30, 40, 2, 3, 4, 200, 300, 400};

    int sum = 0;

    // for_each's second parameter is of type const dze::function<...>&.
    // Hence we know we can safely pass it a lambda that references local
    // variables. There is no way the reference to x will be stored anywhere.
    for_each(vec, [&sum] (const int x) { sum += x; });

    CHECK(sum == 999);
}

TEST_CASE("Ignore return value")
{
    int x = 95;

    // Assign a lambda that return int to a dze::function that returns void.
    dze::function<void()> f = [&] () { return ++x; };

    CHECK(x == 95);
    f();
    CHECK(x == 96);

    dze::function g = [&] () { return ++x; };
    dze::function<void()> cg = std::move(g);

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
        dze::function<double()> f1 = [] () { return 5; };
        CHECK(f1() == 5.0);

        dze::function<int()> f2 = [] () { return 5.2; };
        CHECK(f2() == 5);

        c_derived derived;
        derived.x = 55;

        dze::function<const c_base&()> f3 = [&] () -> const c_derived& { return derived; };
        CHECK(f3().x == 55);

        dze::function<const c_base&()> f4 = [&] () -> c_derived& { return derived; };
        CHECK(f4().x == 55);

        dze::function<c_base&()> f5 = [&] () -> c_derived& { return derived; };
        CHECK(f5().x == 55);

        dze::function<const c_base*()> f6 = [&] () -> const c_derived* { return &derived; };
        CHECK(55 == f6()->x);

        dze::function<const c_base*()> f7 = [&] () -> c_derived* { return &derived; };
        CHECK(f7()->x == 55);

        dze::function<c_base*()> f8 = [&] () -> c_derived* { return &derived; };
        CHECK(f8()->x == 55);

        dze::function<c_base()> f9 = [&] () -> c_derived
        {
            auto d = derived;
            d.x = 66;
            return d;
        };
        CHECK(f9().x == 66);
    }

    SECTION("From function r-value")
    {
        dze::function<int()> f1 = [] () { return 5; };
        dze::function<double()> cf1 = std::move(f1);
        CHECK(cf1() == 5.0);
        dze::function<int()> ccf1 = std::move(cf1);
        CHECK(ccf1() == 5);

        dze::function<double()> f2 = [] () { return 5.2; };
        dze::function<int()> cf2 = std::move(f2);
        CHECK(cf2() == 5);
        dze::function<double()> ccf2 = std::move(cf2);
        CHECK(ccf2() == 5.0);

        c_derived derived;
        derived.x = 55;

        dze::function f3 = [&] () -> const c_derived& { return derived; };
        dze::function<const c_base&()> cf3 = std::move(f3);
        CHECK(cf3().x == 55);

        dze::function f4 = [&] () -> c_derived& { return derived; };
        dze::function<const c_base&()> cf4 = std::move(f4);
        CHECK(cf4().x == 55);

        dze::function f5 = [&] () -> c_derived& { return derived; };
        dze::function<c_base&()> cf5 = std::move(f5);
        CHECK(cf5().x == 55);

        dze::function f6 = [&] () -> const c_derived* { return &derived; };
        dze::function<const c_base*()> cf6 = std::move(f6);
        CHECK(cf6()->x == 55);

        dze::function<const c_derived*()> f7 = [&] () -> c_derived* { return &derived; };
        dze::function<const c_base*()> cf7 = std::move(f7);
        CHECK(cf7()->x == 55);

        dze::function f8 = [&] () -> c_derived* { return &derived; };
        dze::function<c_base*()> cf8 = std::move(f8);
        CHECK(cf8()->x == 55);

        dze::function f9 = [&] () -> c_derived {
            auto d = derived;
            d.x = 66;
            return d;
        };
        dze::function<c_base()> cf9 = std::move(f9);
        CHECK(cf9().x == 66);
    }
}

namespace {

template <typename return_t, typename... Args>
void deduce_args(dze::function<return_t(Args...) const>) {}

} // namespace

TEST_CASE("Deducable arguments")
{
    deduce_args(dze::function{[] {}});
    deduce_args(dze::function{[] (int, float) {}});
    deduce_args(dze::function{[] (int i, float) { return i; }});
}

TEST_CASE("Constructor with copy")
{
    struct X
    {
        X() = default;
        X(const X&) = default;
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

    STATIC_REQUIRE(noexcept(dze::function{lx}));
    STATIC_REQUIRE(!noexcept(dze::function{ly}));
}
