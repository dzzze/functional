#include <array>
#include <functional>
#include <numeric>
#include <random>
#include <iostream>

#include <benchmark/benchmark.h>

#include <dze/function.hpp>

#include "objects.hpp"

constexpr size_t iterations = 4 * 128 * 1024;

int x = 1;

namespace {

void direct_call(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
        benchmark::DoNotOptimize(x += x);
}

void function_pointer(benchmark::State& state)
{
    std::vector<fff*> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_captureless_function();
        benchmark::DoNotOptimize(f(x));
    }
}

void captureless_std_function(benchmark::State& state)
{
    std::vector<std::function<int&(int&)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_captureless_function();
        benchmark::DoNotOptimize(f(x));
    }
}

void captureless_dze_function(benchmark::State& state)
{
    std::vector<dze::function<int&(int&)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_captureless_function();
        benchmark::DoNotOptimize(f(x));
    }
}

void captureless_dze_pmr_function(benchmark::State& state)
{
    std::vector<dze::pmr::function<int&(int&)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_captureless_function();
        benchmark::DoNotOptimize(f(x));
    }
}

void captureless_dze_pmr_function_with_null_memory_resource(benchmark::State& state)
{
    std::vector<dze::pmr::function<int&(int&)>> v;
    v.reserve(iterations);
    for (size_t i = 0; i != v.capacity(); ++i)
        v.emplace_back(std::pmr::null_memory_resource());
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_captureless_function();
        benchmark::DoNotOptimize(f(x));
    }
}

void capture_lambda(benchmark::State& state)
{
    std::vector<capture> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x);
        benchmark::DoNotOptimize(f());
    }
}

void capture_std_function(benchmark::State& state)
{
    std::vector<std::function<int&()>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x);
        benchmark::DoNotOptimize(f());
    }
}

void capture_dze_function(benchmark::State& state)
{
    std::vector<dze::function<int&()>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x);
        benchmark::DoNotOptimize(f());
    }
}

void capture_dze_pmr_function(benchmark::State& state)
{
    std::vector<dze::pmr::function<int&()>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x);
        benchmark::DoNotOptimize(f());
    }
}

void capture_dze_pmr_function_with_null_memory_resource(benchmark::State& state)
{
    std::vector<dze::pmr::function<int&()>> v;
    v.reserve(iterations);
    for (size_t i = 0; i != iterations; ++i)
        v.emplace_back(std::pmr::null_memory_resource());
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x);
        benchmark::DoNotOptimize(f());
    }
}

void random_pick_direct_call(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
    {
        auto nums2 = nums;
        benchmark::DoNotOptimize(x += nums2[rng() % nums.size()]);
    }
}

void random_pick_function_object(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);
    std::vector<capture2> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
    }
}

void random_pick_std_function(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);
    std::vector<std::function<int&(size_t)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
    }
}

void random_pick_dze_function(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);
    std::vector<dze::function<int&(size_t)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
    }
}

void random_pick_dze_function_with_monotonic_buffer_resource(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    std::aligned_storage_t<sizeof(capture2), alignof(capture2)> buf;
    dze::monotonic_buffer_resource mr{&buf, sizeof(buf)};

    std::vector<dze::function<int&(size_t), dze::monotonic_buffer_resource>> v;
    v.reserve(iterations);
    for (size_t i = 0; i != v.capacity(); ++i)
        v.emplace_back(mr);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
    }
}

void random_pick_dze_pmr_function(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);
    std::vector<dze::pmr::function<int&(size_t)>> v(iterations);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
    }
}

void random_pick_dze_pmr_function_with_monotonic_buffer_resource(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    std::aligned_storage_t<sizeof(capture2), alignof(capture2)> buf;
    std::pmr::monotonic_buffer_resource mr{&buf, sizeof(buf), std::pmr::null_memory_resource()};

    std::vector<dze::pmr::function<int&(size_t)>> v;
    v.reserve(iterations);
    for (size_t i = 0; i != v.capacity(); ++i)
        v.emplace_back(&mr);
    auto it = v.begin();

    for ([[maybe_unused]] auto _ : state)
    {
        auto& f = *it++ = get_function_object(x, nums);
        benchmark::DoNotOptimize(f(rng() % nums.size()));
        mr.release();
    }
}

} // namespace

BENCHMARK(direct_call)->Iterations(iterations);
BENCHMARK(function_pointer)->Iterations(iterations);
BENCHMARK(captureless_std_function)->Iterations(iterations);
BENCHMARK(captureless_dze_function)->Iterations(iterations);
BENCHMARK(captureless_dze_pmr_function)->Iterations(iterations);
BENCHMARK(captureless_dze_pmr_function_with_null_memory_resource)->Iterations(iterations);
BENCHMARK(capture_lambda)->Iterations(iterations);
BENCHMARK(capture_std_function)->Iterations(iterations);
BENCHMARK(capture_dze_function)->Iterations(iterations);
BENCHMARK(capture_dze_pmr_function)->Iterations(iterations);
BENCHMARK(capture_dze_pmr_function_with_null_memory_resource)->Iterations(iterations);
BENCHMARK(random_pick_direct_call)->Iterations(iterations);
BENCHMARK(random_pick_function_object)->Iterations(iterations);
BENCHMARK(random_pick_std_function)->Iterations(iterations);
BENCHMARK(random_pick_dze_function)->Iterations(iterations);
BENCHMARK(random_pick_dze_function_with_monotonic_buffer_resource)->Iterations(iterations);
BENCHMARK(random_pick_dze_pmr_function)->Iterations(iterations);
BENCHMARK(random_pick_dze_pmr_function_with_monotonic_buffer_resource)->Iterations(iterations);

BENCHMARK_MAIN();
