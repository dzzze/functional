#include <functional>

#include <dze/function.hpp>

using fff = int&(int&);

fff* get_captureless_function();

inline int& captureless(int& x)
{
    return x += x;
}

fff* get_captureless_function()
{
    return captureless;
}

struct capture
{
    int& x;

    int& operator()();
};

int& capture::operator()() { return x += x; }

capture get_function_object(int&);

capture get_function_object(int& x)
{
    return capture{x};
}

struct capture2
{
    int& x;
    std::array<int, 50> nums;

    int& operator()(size_t);
};

int& capture2::operator()(const size_t idx) { return x += nums[idx]; }

capture2 get_function_object(int&, const std::array<int, 50>&);

capture2 get_function_object(int& x, const std::array<int, 50>& nums)
{
    return capture2{x, nums};
}
