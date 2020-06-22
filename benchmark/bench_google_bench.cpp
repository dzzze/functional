#include <array>
#include <functional>
#include <numeric>
#include <random>
#include <iostream>

#include <benchmark/benchmark.h>

#include <dze/function.hpp>

#include "objects.hpp"

constexpr size_t iterations = 1024;

int x = 1;

namespace {

void directCall(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
        for (size_t i = 0; i != iterations; ++i)
            benchmark::DoNotOptimize(x += x);
}

void functionPointer(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<fff*> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_captureless_function();
            benchmark::DoNotOptimize(f(x));
        }
    }
}

void capturelessStdFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::function<int&(int&)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_captureless_function();
            benchmark::DoNotOptimize(f(x));
        }
    }
}

void capturelessDzeFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::function<int&(int&)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_captureless_function();
            benchmark::DoNotOptimize(f(x));
        }
    }
}

void capturelessDzePmrFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&(int&)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_captureless_function();
            benchmark::DoNotOptimize(f(x));
        }
    }
}

void capturelessDzePmrFunctionWithNullMemoryResource(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&(int&)>> v;
        v.reserve(iterations);
        for (size_t i = 0; i != v.capacity(); ++i)
            v.emplace_back(std::pmr::null_memory_resource());
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_captureless_function();
            benchmark::DoNotOptimize(f(x));
        }
    }
}

void captureLambda(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<capture> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x);
            benchmark::DoNotOptimize(f());
        }
    }
}

void captureStdFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::function<int&()>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x);
            benchmark::DoNotOptimize(f());
        }
    }
}

void captureDzeFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::function<int&()>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x);
            benchmark::DoNotOptimize(f());
        }
    }
}

void captureDzePmrFunction(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&()>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x);
            benchmark::DoNotOptimize(f());
        }
    }
}

void captureDzePmrFunctionWithNullMemoryResource(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&()>> v;
        v.reserve(iterations);
        for (size_t i = 0; i != iterations; ++i)
            v.emplace_back(std::pmr::null_memory_resource());
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x);
            benchmark::DoNotOptimize(f());
        }
    }
}

void randomPickDirectCall(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
        for (size_t i = 0; i != iterations; ++i)
        {
            auto nums2 = nums;
            benchmark::DoNotOptimize(x += nums2[rng() % nums.size()]);
        }
}

void randomPickFunctionObject(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<capture2> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
        }
    }
}

void randomPickStdFunction(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::function<int&(size_t)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
        }
    }
}

void randomPickDzeFunction(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::function<int&(size_t)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
        }
    }
}

void randomPickDzeFunctionWithMonotoniBufferResource(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    std::aligned_storage_t<sizeof(capture2), alignof(capture2)> buf;
    dze::monotonic_buffer_resource mr{&buf, sizeof(buf)};

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::function<int&(size_t), dze::monotonic_buffer_resource>> v;
        v.reserve(iterations);
        for (size_t i = 0; i != v.capacity(); ++i)
            v.emplace_back(mr);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
        }
    }
}

void randomPickDzePmrFunction(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&(size_t)>> v(iterations);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
        }
    }
}

void randomPickDzePmrFunctionWithMonotoniBufferResource(benchmark::State& state)
{
    std::array<int, 64> nums;
    std::mt19937 rng;
    std::generate(nums.begin(), nums.end(), rng);

    std::aligned_storage_t<sizeof(capture2), alignof(capture2)> buf;
    std::pmr::monotonic_buffer_resource mr{&buf, sizeof(buf), std::pmr::null_memory_resource()};

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<dze::pmr::function<int&(size_t)>> v;
        v.reserve(iterations);
        for (size_t i = 0; i != v.capacity(); ++i)
            v.emplace_back(&mr);
        state.ResumeTiming();

        for (auto& f : v)
        {
            f = get_function_object(x, nums);
            benchmark::DoNotOptimize(f(rng() % nums.size()));
            mr.release();
        }
    }
}

} // namespace

BENCHMARK(directCall);
BENCHMARK(functionPointer);
BENCHMARK(capturelessStdFunction);
BENCHMARK(capturelessDzeFunction);
BENCHMARK(capturelessDzePmrFunction);
BENCHMARK(capturelessDzePmrFunctionWithNullMemoryResource);
BENCHMARK(captureLambda);
BENCHMARK(captureStdFunction);
BENCHMARK(captureDzeFunction);
BENCHMARK(captureDzePmrFunction);
BENCHMARK(captureDzePmrFunctionWithNullMemoryResource);
BENCHMARK(randomPickDirectCall);
BENCHMARK(randomPickFunctionObject);
BENCHMARK(randomPickStdFunction);
BENCHMARK(randomPickDzeFunction);
BENCHMARK(randomPickDzeFunctionWithMonotoniBufferResource);
BENCHMARK(randomPickDzePmrFunction);
BENCHMARK(randomPickDzePmrFunctionWithMonotoniBufferResource);

BENCHMARK_MAIN();
