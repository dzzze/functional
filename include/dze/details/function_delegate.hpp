#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

#include <dze/type_traits.hpp>

namespace dze::details::function_ns {

template <typename Callable, bool Const>
auto& get_object(void* const data) noexcept
{
    using callable_decay = std::decay_t<Callable>;
    using cast_to = std::conditional_t<Const, const callable_decay, callable_decay>;

    return *reinterpret_cast<cast_to*>(data);
}

template <
    typename Callable,
    bool Const,
    bool Noexcept,
    typename R,
    typename... Args,
    DZE_REQUIRES(std::is_invocable_r_v<R, Callable, Args...>)>
R call_stub(void* const data, Args... args) noexcept(Noexcept)
{
    if constexpr (std::is_void_v<R>)
        get_object<Callable, Const>(data)(static_cast<Args&&>(args)...);
    else
        return get_object<Callable, Const>(data)(static_cast<Args&&>(args)...);
}

template <typename Callable>
void move_stub(void* const from, void* const to) noexcept
{
    using callable_decay_t = std::decay_t<Callable>;

    ::new (to) callable_decay_t{std::move(get_object<Callable, false>(from))};
}

template <typename Callable>
void deleter_stub(void* const data) noexcept
{
    using callable_decay_t = std::decay_t<Callable>;

    get_object<Callable, false>(data).~callable_decay_t();
}

template <typename, bool>
class delegate_t;

template <bool Noexcept, typename R, typename... Args>
class delegate_t<R(Args...), Noexcept>
{
public:
    delegate_t() = default;

    template <typename Callable, bool Const>
    void set() noexcept
    {
        m_call = call_stub<Callable, Const, Noexcept, R, Args...>;
        m_move = move_stub<Callable>;
        if constexpr (std::is_trivially_destructible_v<std::decay_t<Callable>>)
            m_deleter = nullptr;
        else
            m_deleter = deleter_stub<Callable>;
    }

    void reset() noexcept
    {
        m_call = nullptr;
        m_move = nullptr;
        m_deleter = nullptr;
    }

    void move(void* const from, void* const to) const noexcept
    {
        m_move(from, to);
    }

    void destroy(void* const data) const noexcept
    {
        if (m_deleter != nullptr)
            m_deleter(data);
    }

    [[nodiscard]] bool empty() const noexcept { return m_call == nullptr; }

    R call(const void* const data, Args... args) const noexcept(Noexcept)
    {
        assert(!empty());

        return m_call(const_cast<void*>(data), static_cast<Args&&>(args)...);
    }

    R call(void* data, Args... args) const noexcept(Noexcept)
    {
        assert(!empty());

        return m_call(data, static_cast<Args&&>(args)...);
    }

private:
    using call_t = R(void*, Args...) noexcept(Noexcept);
    using move_t = void(void*, void*) noexcept;
    using deleter_t = void(void*) noexcept;

    call_t* m_call;
    move_t* m_move;
    deleter_t* m_deleter;
};

} // namespace dze::details::function_ns
