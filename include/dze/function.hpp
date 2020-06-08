#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include <dze/type_traits.hpp>

#include "small_buffer.hpp"

namespace dze {

namespace details::function_ns {

template <typename, bool, bool>
class delegate_t;

template <typename Callable, bool Const = false>
// NOLINTNEXTLINE(readability-non-const-parameter)
auto& get_object(std::byte* const data) noexcept
{
    using callable_decay = std::decay_t<Callable>;
    using cast_to = std::conditional_t<Const, const callable_decay, callable_decay>;

    return *reinterpret_cast<cast_to*>(data);
}

template <
    typename Callable,
    bool Const,
    typename R,
    typename... Args,
    DZE_REQUIRES(std::is_invocable_r_v<R, Callable, Args...>)>
R call_stub(std::byte* const data, Args... args)
{
    if constexpr (std::is_void_v<R>)
        get_object<Callable, Const>(data)(static_cast<Args&&>(args)...);
    else
        return get_object<Callable, Const>(data)(static_cast<Args&&>(args)...);
}

template <typename Callable>
void deleter_stub(std::byte* const data) noexcept
{
    using callable_decay_t = std::decay_t<Callable>;

    get_object<Callable>(data).~callable_decay_t();
}

template <bool Const, bool Noexcept, typename R, typename... Args>
class delegate_t<R(Args...), Const, Noexcept>
{
public:
    delegate_t() = default;

    template <bool Const2, bool Noexcept2>
    delegate_t(const delegate_t<R(Args...), Const2, Noexcept2> other) noexcept
        : m_call{reinterpret_cast<decltype(m_call)>(other.m_call)}
        , m_deleter{other.m_deleter} {}

    template <typename Callable>
    void set() noexcept
    {
        m_call = call_stub<Callable, Const, R, Args...>;
        if constexpr (std::is_trivially_destructible_v<std::decay_t<Callable>>)
            m_deleter = nullptr;
        else
            m_deleter = deleter_stub<Callable>;
    }

    void reset() noexcept
    {
        m_call = nullptr;
        m_deleter = nullptr;
    }

    void destroy(std::byte* const data) noexcept
    {
        if (m_deleter != nullptr)
            m_deleter(data);
    }

    [[nodiscard]] bool empty() const noexcept { return m_call == nullptr; }

    template <bool C = Const,
        DZE_REQUIRES(C)>
    R call(const std::byte* const data, Args... args) const noexcept(Noexcept)
    {
        assert(!empty());

        return this->m_call(const_cast<std::byte*>(data), static_cast<Args&&>(args)...);
    }

    template <bool C = Const,
        DZE_REQUIRES(!C)>
    R call(std::byte* data, Args... args) noexcept(Noexcept)
    {
        assert(!empty());

        return this->m_call(data, static_cast<Args&&>(args)...);
    }

private:
    template <typename, bool, bool>
    friend class delegate_t;

    using call_t = R(std::byte*, Args...);
    using deleter_t = void(std::byte*) noexcept;

    call_t* m_call;
    deleter_t* m_deleter;
};

template <size_t Size, size_t Align, typename Alloc>
class storage
{
public:
    storage() noexcept = default;

    storage(const Alloc& alloc) noexcept
        : m_underlying{alloc} {}

    storage(const size_t size, const Alloc& alloc) noexcept
        : m_underlying{size, alloc} {}

    storage(const size_t size, const size_t alignment, const Alloc& alloc) noexcept
        : m_underlying{size, alignment, alloc} {}

    void swap(storage& other) noexcept { m_underlying.swap(other.m_underlying); }

    void resize(const size_t size, const size_t alignment) noexcept
    {
        m_underlying.resize_discard(size, alignment);
    }

    [[nodiscard]] size_t size() const noexcept { return m_underlying.size(); }

    [[nodiscard]] const std::byte* data() const noexcept { return m_underlying.data(); }

    [[nodiscard]] std::byte* data() noexcept { return m_underlying.data(); }

private:
    small_buffer<Size, Align, Alloc> m_underlying;
};

template <typename From, typename To>
inline constexpr bool is_safely_convertible_v =
    !std::is_reference_v<To> || std::is_reference_v<From>;

template <typename, typename>
class base;

template <typename Function, typename R, typename... Args>
class base<Function, R(Args...)>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args)
    {
        return static_cast<Function*>(this)->m_delegate.call(
            static_cast<Function*>(this)->data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            std::is_invocable_v<std::decay_t<Callable>&, Args...> &&
            is_safely_convertible_v<
                std::invoke_result_t<std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), false, false>;
    using const_signature = R(Args...) const;
    using nothrow_signature = R(Args...) noexcept;
    using const_nothrow_signature = R(Args...) const noexcept;

    template <typename Callable>
    static constexpr bool is_convertible_v = is_convertible<Callable>::value;
};

template <typename Function, typename R, typename... Args>
class base<Function, R(Args...) const>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args) const
    {
        return static_cast<const Function*>(this)->m_delegate.call(
            static_cast<const Function*>(this)->data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            std::is_invocable_v<const std::decay_t<Callable>&, Args...> &&
            is_safely_convertible_v<
                std::invoke_result_t<const std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), true, false>;
    using const_signature = R(Args...) const;
    using nothrow_signature = R(Args...) const noexcept;
    using const_nothrow_signature = R(Args...) const noexcept;

    template <typename Callable>
    static constexpr bool is_convertible_v = is_convertible<Callable>::value;
};

template <typename Function, typename R, typename... Args>
class base<Function, R(Args...) noexcept>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args) noexcept
    {
        return static_cast<Function*>(this)->m_delegate.call(
            static_cast<Function*>(this)->data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            std::is_nothrow_invocable_v<std::decay_t<Callable>&, Args...> &&
            is_safely_convertible_v<
                std::invoke_result_t<std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), false, true>;
    using const_signature = R(Args...) const noexcept;
    using nothrow_signature = R(Args...) noexcept;
    using const_nothrow_signature = R(Args...) const noexcept;

    template <typename Callable>
    static constexpr bool is_convertible_v = is_convertible<Callable>::value;
};

template <typename Function, typename R, typename... Args>
class base<Function, R(Args...) const noexcept>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args) const noexcept
    {
        return static_cast<const Function*>(this)->m_delegate.call(
            static_cast<const Function*>(this)->data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            std::is_nothrow_invocable_v<const std::decay_t<Callable>&, Args...> &&
            is_safely_convertible_v<
                std::invoke_result_t<const std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), true, true>;
    using const_signature = R(Args...) const noexcept;
    using nothrow_signature = R(Args...) const noexcept;
    using const_nothrow_signature = R(Args...) const noexcept;

    template <typename Callable>
    static constexpr bool is_convertible_v = is_convertible<Callable>::value;
};

} // namespace details::function_ns

template <typename, typename>
class function;

template <typename>
struct is_function : std::false_type {};

template <typename Signature, typename Alloc>
struct is_function<function<Signature, Alloc>> : std::true_type {};

template <typename T>
inline constexpr bool is_function_v = is_function<T>::value;

// Move-only polymorphic function wrapper.
// XXX: This object assumes the function objects it stores are trivially
// relocatable for move and swap operations. As of C++17 standard, there is no
// avaiable way to detect this trait for types. Therefore, it is up to the user
// to correctly pass a trivially relocatable type for correct operation. It is
// undefined behavior to pass a non-trivially-relocatable type and use the move
// constructor or swap method.
template <typename Signature, typename Alloc = aligned_allocator<std::byte>>
class function : public details::function_ns::base<function<Signature, Alloc>, Signature>
{
    using base = details::function_ns::base<function, Signature>;

    struct conv_tag_t {};

    template <typename Sig>
    static constexpr bool is_movable_v =
        std::is_same_v<Sig, Signature> ||
        std::is_same_v<Sig, typename base::const_signature> ||
        std::is_same_v<Sig, typename base::nothrow_signature> ||
        std::is_same_v<Sig, typename base::const_nothrow_signature>;

public:
    function() noexcept
        : function{Alloc{}} {}

    function(const Alloc& alloc) noexcept
        : m_storage{alloc}
    {
        m_delegate.reset();
    }

    function(std::nullptr_t, const Alloc& alloc = Alloc{}) noexcept
        : function{alloc} {}

    template <typename Callable,
        DZE_REQUIRES(!is_function_v<Callable> && base::template is_convertible_v<Callable>)>
    function(Callable call, const Alloc& alloc = Alloc{}) noexcept
        : function{std::move(call), alloc, conv_tag_t{}} {}

    template <
        typename Member,
        typename Object,
        typename = decltype(function{std::mem_fn(std::declval<Member Object::*>())})>
    // NOLINTNEXTLINE(readability-avoid-const-params-in-decls)
    function(Member Object::*const ptr, const Alloc& alloc = Alloc{}) noexcept
        : function{alloc}
    {
        if (ptr)
            *this = std::mem_fn(ptr);
    }

    function(const function&) = delete;
    function& operator=(const function&) = delete;

    template <typename Signature2 = Signature,
        DZE_REQUIRES(is_movable_v<Signature2>)>
    function(function<Signature2, Alloc>&& other) noexcept
        : m_delegate{other.m_delegate}
        , m_storage{std::move(other.m_storage)}
    {
        other.m_delegate.reset();
    }

    template <
        typename Signature2 = Signature,
        typename Alloc2 = Alloc,
        DZE_REQUIRES(
            !is_movable_v<Signature2> &&
            base::template is_convertible_v<function<Signature2>>)>
    function(function<Signature2, Alloc2>&& other, const Alloc& alloc = Alloc{}) noexcept
        : function{std::move(other), alloc, conv_tag_t{}} {}

    template <typename Signature2 = Signature,
        DZE_REQUIRES(is_movable_v<Signature2>)>
    function& operator=(function<Signature2>&& other) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate = other.m_delegate;
        m_storage = std::move(other.m_storage);
        other.m_delegate.reset();
        return *this;
    }

    template <typename Signature2 = Signature, typename Alloc2 = Alloc,
        DZE_REQUIRES(
            !is_movable_v<Signature2> &&
            base::template is_convertible_v<function<Signature2>>)>
    function& operator=(function<Signature2, Alloc2>&& other) noexcept
    {
        assign(std::move(other));
        return *this;
    }

    ~function() { m_delegate.destroy(data_addr()); }

    void swap(function& other) noexcept
    {
        std::swap(m_delegate, other.m_delegate);
        m_storage.swap(other.m_storage);
    }

    function& operator=(std::nullptr_t) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate.reset();
        return *this;
    }

    template <typename Callable,
        DZE_REQUIRES(!is_function_v<Callable> && base::template is_convertible_v<Callable>)>
    function& operator=(Callable call) noexcept
    {
        assign(std::move(call));
        return *this;
    }

    template <
        typename Member,
        typename Object,
        typename = decltype(function{std::mem_fn(std::declval<Member Object::*>())})>
    function& operator=(Member Object::*const ptr) noexcept
    {
        *this = ptr ? std::mem_fn(ptr) : nullptr;
        return *this;
    }

    explicit operator bool() const noexcept { return !m_delegate.empty(); }

private:
    friend class details::function_ns::base<function, Signature>;

    template <typename, typename>
    friend class function;

    friend bool operator==(const function& f, std::nullptr_t) noexcept
    {
        return !f;
    }

    friend bool operator==(std::nullptr_t, const function& f) noexcept
    {
        return !f;
    }

    friend bool operator!=(const function& f, std::nullptr_t) noexcept
    {
        return static_cast<bool>(f);
    }

    friend bool operator!=(std::nullptr_t, const function& f) noexcept
    {
        return static_cast<bool>(f);
    }

    alignas(32) typename base::delegate_type m_delegate;
    details::function_ns::storage<128 - sizeof(m_delegate), alignof(std::max_align_t), Alloc>
        m_storage;

    template <typename Callable>
    function(Callable call, const Alloc& alloc, conv_tag_t) noexcept
        : m_storage{sizeof(std::decay_t<Callable>), alignof(std::decay_t<Callable>), alloc}
    {
        m_delegate.template set<Callable>();
        ::new (data_addr()) std::decay_t<Callable>{std::move(call)};
    }

    template <typename Callable>
    void assign(Callable call) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate.template set<Callable>();
        m_storage.resize(sizeof(std::decay_t<Callable>), alignof(std::decay_t<Callable>));
        ::new (data_addr()) std::decay_t<Callable>{std::move(call)};
    }

    [[nodiscard]] const std::byte* data_addr() const noexcept { return m_storage.data(); }

    [[nodiscard]] std::byte* data_addr() noexcept { return m_storage.data(); }
};

template <typename R, typename... Args, typename Alloc = aligned_allocator<std::byte>>
function(R (*)(Args...), Alloc = Alloc{})
    -> function<R(Args...), Alloc>;

template <typename R, typename... Args, typename Alloc = aligned_allocator<std::byte>>
function(R (*)(Args...) noexcept, Alloc = Alloc{})
    -> function<R(Args...) noexcept, Alloc>;

namespace details::function_ns {

template <typename>
struct guide_helper {};

template <typename R, typename T, bool Noexcept, typename... Args>
struct guide_helper<R(T::*)(Args...) noexcept(Noexcept)>
{
    using type = R(Args...) noexcept(Noexcept);
};

template <typename R, typename T, bool Noexcept, typename... Args>
struct guide_helper<R(T::*)(Args...) & noexcept(Noexcept)>
{
    using type = R(Args...) noexcept(Noexcept);
};

template <typename R, typename T, bool Noexcept, typename... Args>
struct guide_helper<R(T::*)(Args...) const noexcept(Noexcept)>
{
    using type = R(Args...) const noexcept(Noexcept);
};

template <typename R, typename T, bool Noexcept, typename... Args>
struct guide_helper<R(T::*)(Args...) const& noexcept(Noexcept)>
{
    using type = R(Args...) const noexcept(Noexcept);
};

template <typename Callable>
using guide_helper_t = typename guide_helper<decltype(&Callable::operator())>::type;

} // namespace details::function_ns

template <
    typename Callable,
    typename Signature = details::function_ns::guide_helper_t<Callable>,
    typename Alloc = aligned_allocator<std::byte>>
function(Callable, Alloc = Alloc{}) -> function<Signature, Alloc>;

} // namespace dze
