#include <functional>

#include "objects.hpp"

inline int& captureless(int& x)
{
    return x += x;
}

fff* get_captureless_function()
{
    return captureless;
}

int& capture::operator()() const { return *x += *x; }

capture get_function_object(int& x)
{
    return capture{&x};
}

int& capture2::operator()(const size_t idx) { return *x += nums[idx]; }

capture2 get_function_object(int& x, const std::array<int, 64>& nums)
{
    return capture2{&x, nums};
}
