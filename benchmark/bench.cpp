#include <array>
#include <functional>
#include <numeric>
#include <random>

#include <nanobench.h>

#include <dze/function.hpp>

int main()
{
    uint64_t x = 1;

    ankerl::nanobench::Bench().run(
        "direct: x += x", [&] () { ankerl::nanobench::doNotOptimizeAway(x += x); });

    ankerl::nanobench::Bench().run(
        "lambda: x += x",
        [&] ()
        {
            auto f = [&] { return x += x; };
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    ankerl::nanobench::Bench().run(
        "std::function: x += x",
        [&] ()
        {
            std::function f = [&] { return x += x; };
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    ankerl::nanobench::Bench().run(
        "dze::function: x += x",
        [&] ()
        {
            dze::function f = [&] { return x += x; };
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    ankerl::nanobench::Bench().run(
        "dze::pmr::function: x += x",
        [&] ()
        {
            dze::pmr::function<int()> f = [&] { return x += x; };
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    {
        std::byte buf[sizeof(int&)];
        ankerl::nanobench::Bench().run(
            "dze::pmr::function with monotonic_buffer_resource: x += x",
            [&] ()
            {
                std::pmr::monotonic_buffer_resource mr{buf, sizeof(buf), std::pmr::null_memory_resource()};
                dze::pmr::function<int()> f{[&] { return x += x; }, &mr};
                ankerl::nanobench::doNotOptimizeAway(f());
            });
    }

    ankerl::nanobench::Bench().run(
        "captureless lambda: x += x",
        [&] ()
        {
            auto f = [] (int y) { return y += y; };
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    ankerl::nanobench::Bench().run(
        "captureless std::function: x += x",
        [&] ()
        {
            std::function f = [] (int y) { return y += y; };
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    ankerl::nanobench::Bench().run(
        "captureless dze::function: x += x",
        [&] ()
        {
            dze::function f = [] (int y) { return y += y; };
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    ankerl::nanobench::Bench().run(
        "captureless dze::pmr::function: x += x",
        [&] ()
        {
            dze::pmr::function<int(int)> f = [] (int y) { return y += y; };
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    {
        std::byte buf[1];
        ankerl::nanobench::Bench().run(
            "captureless dze::pmr::function with monotonic_buffer_resource: x += x",
            [&] ()
            {
                std::pmr::monotonic_buffer_resource mr{buf, 0, std::pmr::null_memory_resource()};
                dze::pmr::function<int(int)> f{[] (int y) { return y += y; }, &mr};
                ankerl::nanobench::doNotOptimizeAway(f(x));
            });
    }

    std::array<int, 100000> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    ankerl::nanobench::Bench().run(
        "direct: random pick",
        [&] ()
        {
            auto nums2 = nums;
            ankerl::nanobench::doNotOptimizeAway(++nums2[rng() % nums.size()]);
        });

    ankerl::nanobench::Bench().run(
        "lambda: random pick",
        [&] ()
        {
            auto f = [nums] (const size_t idx) mutable { return ++nums[idx]; };
            ankerl::nanobench::doNotOptimizeAway(f(rng() % nums.size()));
        });

    ankerl::nanobench::Bench().run(
        "std::function: random pick",
        [&] ()
        {
            std::function f = [nums] (const size_t idx) mutable { return ++nums[idx]; };
            ankerl::nanobench::doNotOptimizeAway(f(rng() % nums.size()));
        });

    ankerl::nanobench::Bench().run(
        "dze::function: random pick",
        [&] ()
        {
            dze::function f = [nums] (const size_t idx) mutable { return ++nums[idx]; };
            ankerl::nanobench::doNotOptimizeAway(f(rng() % nums.size()));
        });

    ankerl::nanobench::Bench().run(
        "dze::pmr::function: random pick",
        [&] ()
        {
            dze::pmr::function<int(size_t)> f =
                [nums] (const size_t idx) mutable { return ++nums[idx]; };
            ankerl::nanobench::doNotOptimizeAway(f(rng() % nums.size()));
        });

    {
        std::byte buf[nums.size() * sizeof(int)];
        ankerl::nanobench::Bench().run(
            "dze::pmr::function with monotonic_buffer_resource: random pick",
            [&] ()
            {
                std::pmr::monotonic_buffer_resource mr{buf, sizeof(buf), std::pmr::null_memory_resource()};
                dze::pmr::function<int(size_t)> f{
                    [nums] (const size_t idx) mutable { return ++nums[idx]; }, &mr};
                ankerl::nanobench::doNotOptimizeAway(f(rng() % nums.size()));
            });
    }
}
