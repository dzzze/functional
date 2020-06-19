#include <array>
#include <functional>
#include <numeric>
#include <random>
#include <iostream>

#include <nanobench.h>

#include <dze/function.hpp>

using fff = int&(int&);

fff* get_captureless_function();

struct capture
{
    int& x;

    int& operator()();
};

capture get_function_object(int&);

struct capture2
{
    int& x;
    std::array<int, 50> nums;

    int& operator()(size_t);
};

capture2 get_function_object(int&, const std::array<int, 50>&);

int main()
{
    int x = 1;

    auto bench = ankerl::nanobench::Bench();
    bench.title("x += x, captureless");

    bench.run(
        "direct call", [&] () { ankerl::nanobench::doNotOptimizeAway(x += x); });

    bench.run(
        "lambda",
        [&] ()
        {
            auto f = get_captureless_function();
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    bench.run(
        "std::function",
        [&] ()
        {
            std::function f = get_captureless_function();
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    bench.run(
        "dze::function",
        [&] ()
        {
            auto f = get_captureless_function();
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    bench.run(
        "dze::pmr::function",
        [&] ()
        {
            dze::pmr::function<int&(int&)> f = get_captureless_function();
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    bench.run(
        "dze::pmr::function with null_memory_resource",
        [&] ()
        {
            dze::pmr::function<int&(int&)> f{get_captureless_function(), std::pmr::null_memory_resource()};
            ankerl::nanobench::doNotOptimizeAway(f(x));
        });

    bench.title("x += x");

    bench.run(
        "direct call", [&] () { ankerl::nanobench::doNotOptimizeAway(x += x); });

    bench.run(
        "lambda",
        [&] ()
        {
            auto f = get_function_object(x);
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    bench.run(
        "std::function",
        [&] ()
        {
            std::function f = get_function_object(x);
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    bench.run(
        "dze::function",
        [&] ()
        {
            dze::function f = get_function_object(x);
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    bench.run(
        "dze::pmr::function",
        [&] ()
        {
            dze::pmr::function<int&()> f = get_function_object(x);
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    bench.run(
        "dze::pmr::function with null_memory_resource",
        [&] ()
        {
            dze::pmr::function<int&()> f{get_function_object(x), std::pmr::null_memory_resource()};
            ankerl::nanobench::doNotOptimizeAway(f());
        });

    bench.title("random pick");

    std::array<int, 50> nums;
    std::generate(nums.begin(), nums.end(), ankerl::nanobench::Rng{});
    ankerl::nanobench::Rng rng{0};

    bench.run(
        "direct call",
        [&] ()
        {
            auto nums2 = nums;
            ankerl::nanobench::doNotOptimizeAway( x += nums2[rng.bounded(nums.size())]);
        });

    rng = ankerl::nanobench::Rng{0};

    {
        // TODO auto f = [nums] (const size_t idx) mutable { return ++nums[idx]; };
        bench.run(
            "lambda",
            [&] ()
            {
                auto f = get_function_object(x, nums);
                ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
            });
    }

    rng = ankerl::nanobench::Rng{0};

    bench.run(
        "std::function",
        [&] ()
        {
            std::function f = get_function_object(x, nums);
            ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
            //f = nullptr;
            //ankerl::nanobench::doNotOptimizeAway(!f);
            //f = get_function_object(x, nums);
            //ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
        });

    rng = ankerl::nanobench::Rng{0};

    // There could be seveal reasons that these benchmarks do no show the difference in speed.
    // Allocation and deallocation in a tight loop may be very optimized.
    // Copying of the data may be more costly than the allocations.
    // When it does allocate, dze::function does not have a significant advantage over std::function.
    bench.run(
        "dze::function",
        [&] ()
        {
            dze::function f = get_function_object(x, nums);
            ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
            //f = nullptr;
            //ankerl::nanobench::doNotOptimizeAway(!f);
            //f = get_function_object(x, nums);
            //ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
        });

    rng = ankerl::nanobench::Rng{0};

    bench.run(
        "dze::pmr::function",
        [&] ()
        {
            dze::pmr::function<int(size_t)> f = get_function_object(x, nums);
            ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
        });

    rng = ankerl::nanobench::Rng{0};

    {
        //auto lambda = [&x, nums] (const size_t idx) mutable { return x += nums[idx]; };
        std::aligned_storage_t<sizeof(capture2) + 64, alignof(capture2)> buf;
        //std::cerr << sizeof(lambda) << " " << sizeof(buf) << " " << alignof(decltype(lambda)) << " " << alignof(decltype(buf)) << std::endl;
        bench.run(
            "dze::pmr::function with monotonic_buffer_resource",
            [&] ()
            {
                std::pmr::monotonic_buffer_resource mr{&buf, sizeof(buf), std::pmr::null_memory_resource()};
                dze::pmr::function<int(size_t)> f{get_function_object(x, nums), &mr};
                ankerl::nanobench::doNotOptimizeAway(f(rng.bounded(nums.size())));
            });
    }
}
