#pragma once

#include <array>
#include <functional>

using fff = int&(int&);

fff* get_captureless_function();

struct capture
{
    int* x;

    int& operator()() const;
};

capture get_function_object(int&);

struct capture2
{
    int* x;
    alignas(64) std::array<int, 64> nums;

    int& operator()(size_t);
};

capture2 get_function_object(int&, const std::array<int, 64>&);
