#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

#include <dze/type_traits.hpp>

#include "small_buffer.hpp"

namespace q::util {

namespace details::function_ns {

template <typename, bool, bool>
class delegate_t;

template <typename callable, bool is_const = false>
// NOLINTNEXTLINE(readability-non-const-parameter)
auto& get_object(char* const data) noexcept
{
    using callable_decay = std::decay_t<callable>;
    using cast_to = std::conditional_t<is_const, const callable_decay, callable_decay>;

    return *reinterpret_cast<cast_to*>(data);
}

template <
    typename callable,
    bool is_const,
    typename return_t,
    typename... args_t,
    DZE_REQUIRES(std::is_invocable_r_v<return_t, callable, args_t...>)>
return_t call_stub(char* const data, args_t... args)
{
    if constexpr (std::is_void_v<return_t>)
        get_object<callable, is_const>(data)(static_cast<args_t&&>(args)...);
    else
        return get_object<callable, is_const>(data)(static_cast<args_t&&>(args)...);
}

template <typename callable>
void deleter_stub(char* const data) noexcept
{
    using callable_decay_t = std::decay_t<callable>;

    get_object<callable>(data).~callable_decay_t();
}

template <bool is_const, bool is_noexcept, typename return_t, typename... args_t>
class delegate_t<return_t(args_t...), is_const, is_noexcept>
{
public:
    delegate_t() = default;

    template <bool o_is_const, bool o_is_noexcept>
    delegate_t(const delegate_t<return_t(args_t...), o_is_const, o_is_noexcept> other) noexcept
        : m_call{reinterpret_cast<decltype(m_call)>(other.m_call)}
        , m_deleter{other.m_deleter} {}

    template <typename callable>
    void set() noexcept
    {
        m_call = call_stub<callable, is_const, return_t, args_t...>;
        if constexpr (std::is_trivially_destructible_v<std::decay_t<callable>>)
            m_deleter = nullptr;
        else
            m_deleter = deleter_stub<callable>;
    }

    void reset() noexcept
    {
        m_call = nullptr;
        m_deleter = nullptr;
    }

    void destroy(char* const data) noexcept
    {
        if (m_deleter != nullptr)
            m_deleter(data);
    }

    [[nodiscard]] bool empty() const noexcept { return m_call == nullptr; }

    template <bool dummy = is_const,
        DZE_REQUIRES(dummy)>
    return_t call(const char* const data, args_t... args) const noexcept(is_noexcept)
    {
        assert(this->m_call);

        return this->m_call(const_cast<char*>(data), static_cast<args_t&&>(args)...);
    }

    template <bool dummy = is_const,
        DZE_REQUIRES(!dummy)>
    return_t call(char* data, args_t... args) noexcept(is_noexcept)
    {
        assert(this->m_call);

        return this->m_call(data, static_cast<args_t&&>(args)...);
    }

private:
    template <typename, bool, bool>
    friend class delegate_t;

    using call_t = return_t(char*, args_t...);
    using deleter_t = void(char*) noexcept;

    call_t* m_call;
    deleter_t* m_deleter;
};

template <typename allocator>
class storage
{
public:
    storage() noexcept = default;

    storage(const allocator& alloc) noexcept
        : m_storage{alloc} {}

    storage(const size_t size, const allocator& alloc) noexcept
        : m_storage{size, alloc} {}

    storage(const size_t size, const size_t alignment, const allocator& alloc) noexcept
        : m_storage{size, alignment, alloc} {}

    storage(const storage&) = delete;
    storage& operator=(const storage&) = delete;

    storage(storage&&) noexcept = default;
    storage& operator=(storage&&) noexcept = default;

    void swap(storage& other) noexcept { m_storage.swap(other.m_storage); }

    void resize(const size_t size, const size_t alignment) noexcept
    {
        m_storage.resize_discard(size, alignment);
    }

    [[nodiscard]] size_t size() const noexcept { return m_storage.size(); }

    [[nodiscard]] const char* data() const noexcept { return m_storage.data(); }

    [[nodiscard]] char* data() noexcept { return m_storage.data(); }

private:
    small_buffer<128 - 2 * sizeof(void*), 2 * alignof(void*), allocator> m_storage;
};

template <typename from, typename to>
inline constexpr bool is_safely_convertible_v =
    !std::is_reference_v<to> || std::is_reference_v<from>;

template <typename, typename>
class base;

template <typename function_t, typename return_t, typename... args_t>
class base<function_t, return_t(args_t...)>
{
public:
    // Pre-condition: A call is stored in this object.
    return_t operator()(args_t... args)
    {
        return static_cast<function_t*>(this)->m_delegate.call(
            static_cast<function_t*>(this)->data_addr(), static_cast<args_t&&>(args)...);
    }

private:
    template <typename callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename callable>
    struct is_convertible<
        callable,
        std::enable_if_t<
            is_safely_convertible_v<
                std::invoke_result_t<std::decay_t<callable>&, args_t...>, return_t> &&
            std::is_invocable_v<std::decay_t<callable>&, args_t...>>>
        : std::true_type {};

    static constexpr bool is_const = false;
    static constexpr bool is_noexcept = false;

protected:
    using delegate_type = delegate_t<return_t(args_t...), is_const, is_noexcept>;
    using const_signature = return_t(args_t...) const;
    using nothrow_signature = return_t(args_t...) noexcept;
    using const_nothrow_signature = return_t(args_t...) const noexcept;

    template <typename callable>
    static constexpr bool is_convertible_v = is_convertible<callable>::value;
};

template <typename function_t, typename return_t, typename... args_t>
class base<function_t, return_t(args_t...) const>
{
public:
    // Pre-condition: A call is stored in this object.
    return_t operator()(args_t... args) const
    {
        return static_cast<const function_t*>(this)->m_delegate.call(
            static_cast<const function_t*>(this)->data_addr(), static_cast<args_t&&>(args)...);
    }

private:
    template <typename callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename callable>
    struct is_convertible<
        callable,
        std::enable_if_t<
            is_safely_convertible_v<
                std::invoke_result_t<const std::decay_t<callable>&, args_t...>, return_t> &&
            std::is_invocable_r_v<return_t, const std::decay_t<callable>&, args_t...>>>
        : std::true_type {};

    static constexpr bool is_const = true;
    static constexpr bool is_noexcept = false;

protected:
    using delegate_type = delegate_t<return_t(args_t...), is_const, is_noexcept>;
    using const_signature = return_t(args_t...) const;
    using nothrow_signature = return_t(args_t...) const noexcept;
    using const_nothrow_signature = return_t(args_t...) const noexcept;

    template <typename callable>
    static constexpr bool is_convertible_v = is_convertible<callable>::value;
};

template <typename function_t, typename return_t, typename... args_t>
class base<function_t, return_t(args_t...) noexcept>
{
public:
    // Pre-condition: A call is stored in this object.
    return_t operator()(args_t... args) noexcept
    {
        return static_cast<function_t*>(this)->m_delegate.call(
            static_cast<function_t*>(this)->data_addr(), static_cast<args_t&&>(args)...);
    }

private:
    template <typename callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename callable>
    struct is_convertible<
        callable,
        std::enable_if_t<
            is_safely_convertible_v<
                std::invoke_result_t<std::decay_t<callable>&, args_t...>, return_t> &&
            std::is_nothrow_invocable_v<std::decay_t<callable>&, args_t...>>>
        : std::true_type {};

    static constexpr bool is_const = false;
    static constexpr bool is_noexcept = true;

protected:
    using delegate_type = delegate_t<return_t(args_t...), is_const, is_noexcept>;
    using const_signature = return_t(args_t...) const noexcept;
    using nothrow_signature = return_t(args_t...) noexcept;
    using const_nothrow_signature = return_t(args_t...) const noexcept;

    template <typename callable>
    static constexpr bool is_convertible_v = is_convertible<callable>::value;
};

template <typename function_t, typename return_t, typename... args_t>
class base<function_t, return_t(args_t...) const noexcept>
{
public:
    // Pre-condition: A call is stored in this object.
    return_t operator()(args_t... args) const noexcept
    {
        return static_cast<const function_t*>(this)->m_delegate.call(
            static_cast<const function_t*>(this)->data_addr(), static_cast<args_t&&>(args)...);
    }

private:
    template <typename callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename callable>
    struct is_convertible<
        callable,
        std::enable_if_t<
            is_safely_convertible_v<
                std::invoke_result_t<const std::decay_t<callable>&, args_t...>,
                return_t> &&
            std::is_nothrow_invocable_v<const std::decay_t<callable>&, args_t...>>>
        : std::true_type {};

    static constexpr bool is_const = true;
    static constexpr bool is_noexcept = true;

protected:
    using delegate_type = delegate_t<return_t(args_t...), is_const, is_noexcept>;
    using const_signature = return_t(args_t...) const noexcept;
    using nothrow_signature = return_t(args_t...) const noexcept;
    using const_nothrow_signature = return_t(args_t...) const noexcept;

    template <typename callable>
    static constexpr bool is_convertible_v = is_convertible<callable>::value;
};

} // namespace details::function_ns

template <typename, typename>
class function;

template <typename>
struct is_function : std::false_type {};

template <typename signature, typename allocator>
struct is_function<function<signature, allocator>> : std::true_type {};

template <typename T>
inline constexpr bool is_function_v = is_function<T>::value;

// Move-only polymorphic function wrapper.
// XXX: This object assumes the function objects it stores are trivially
// relocatable for move and swap operations. As of C++17 standard, there is no
// avaiable way to detect this trait for types. Therefore, it is up to the user
// to correctly pass a trivially relocatable type for correct operation. It is
// undefined behavior to pass a non-trivially-relocatable type and use the move
// constructor or swap method.
template <typename signature, typename allocator = aligned_allocator<char>>
class function : public details::function_ns::base<function<signature, allocator>, signature>
{
    using base = details::function_ns::base<function, signature>;

    struct conv_tag_t {};

    template <typename other_signature>
    static constexpr bool is_movable_v =
        std::is_same_v<other_signature, typename base::const_signature> ||
        std::is_same_v<other_signature, typename base::nothrow_signature> ||
        std::is_same_v<other_signature, typename base::const_nothrow_signature>;

public:
    function() noexcept
        : function{allocator{}} {}

    function(const allocator& alloc) noexcept
        : m_storage{alloc}
    {
        m_delegate.reset();
    }

    function(std::nullptr_t, const allocator& alloc = allocator{}) noexcept
        : function{alloc} {}

    template <typename callable,
        DZE_REQUIRES(!is_function_v<callable> && base::template is_convertible_v<callable>)>
    function(callable call, const allocator& alloc = allocator{}) noexcept
        : function{std::move(call), alloc, conv_tag_t{}} {}

    template <
        typename member_t,
        typename object_t,
        typename = decltype(function{std::mem_fn(std::declval<member_t object_t::*>())})>
    // NOLINTNEXTLINE(readability-avoid-const-params-in-decls)
    function(member_t object_t::*const ptr, const allocator& alloc = allocator{}) noexcept
        : function{alloc}
    {
        if (ptr)
            *this = std::mem_fn(ptr);
    }

    function(const function&) = delete;
    function& operator=(const function&) = delete;

    function(function&& other) noexcept
        : m_delegate{other.m_delegate}
        , m_storage{std::move(other.m_storage)}
    {
        other.m_delegate.reset();
    }

    template <typename other_signature = signature,
        DZE_REQUIRES(is_movable_v<other_signature>)>
    function(function<other_signature, allocator>&& other) noexcept
        : m_delegate{other.m_delegate}
        , m_storage{std::move(other.m_storage)}
    {
        other.m_delegate.reset();
    }

    template <
        typename other_signature = signature,
        typename other_alloc = allocator,
        DZE_REQUIRES(
            !is_movable_v<other_signature> &&
            base::template is_convertible_v<function<other_signature>>)>
    function(function<other_signature, other_alloc>&& other, const allocator& alloc = allocator{}) noexcept
        : function{std::move(other), alloc, conv_tag_t{}} {}

    function& operator=(function&& other) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate = other.m_delegate;
        m_storage = std::move(other.m_storage);
        other.m_delegate.reset();
        return *this;
    }

    template <typename other_signature = signature,
        DZE_REQUIRES(is_movable_v<other_signature>)>
    function& operator=(function<other_signature>&& other) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate = other.m_delegate;
        m_storage = std::move(other.m_storage);
        other.m_delegate.reset();
        return *this;
    }

    template <typename other_signature = signature, typename other_alloc = allocator,
        DZE_REQUIRES(
            !is_movable_v<other_signature> &&
            base::template is_convertible_v<function<other_signature>>)>
    function& operator=(function<other_signature, other_alloc>&& other) noexcept
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

    template <typename callable,
        DZE_REQUIRES(!is_function_v<callable> && base::template is_convertible_v<callable>)>
    function& operator=(callable call) noexcept
    {
        assign(std::move(call));
        return *this;
    }

    template <
        typename member_t,
        typename object_t,
        typename = decltype(function{std::mem_fn(std::declval<member_t object_t::*>())})>
    function& operator=(member_t object_t::*const ptr) noexcept
    {
        *this = ptr ? std::mem_fn(ptr) : nullptr;
        return *this;
    }

    explicit operator bool() const noexcept { return !m_delegate.empty(); }

private:
    friend class details::function_ns::base<function, signature>;

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

    alignas(64) typename base::delegate_type m_delegate;
    details::function_ns::storage<allocator> m_storage;

    template <typename callable>
    function(callable call, const allocator& alloc, conv_tag_t) noexcept
        : m_storage{sizeof(std::decay_t<callable>), alignof(std::decay_t<callable>), alloc}
    {
        m_delegate.template set<callable>();
        ::new (data_addr()) std::decay_t<callable>{std::move(call)};
    }

    template <typename callable>
    void assign(callable call) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate.template set<callable>();
        m_storage.resize(sizeof(std::decay_t<callable>), alignof(std::decay_t<callable>));
        ::new (data_addr()) std::decay_t<callable>{std::move(call)};
    }

    [[nodiscard]] const char* data_addr() const noexcept { return m_storage.data(); }

    [[nodiscard]] char* data_addr() noexcept { return m_storage.data(); }
};

template <typename return_t, typename... args_t, typename allocator = aligned_allocator<char>>
function(return_t (*)(args_t...), allocator = allocator{})
    -> function<return_t(args_t...), allocator>;

template <typename return_t, typename... args_t, typename allocator = aligned_allocator<char>>
function(return_t (*)(args_t...) noexcept, allocator = allocator{})
    -> function<return_t(args_t...) noexcept, allocator>;

namespace details::function_ns {

template <typename>
struct guide_helper {};

template <typename return_t, typename T, bool is_noexcept, typename... args_t>
struct guide_helper<return_t (T::*)(args_t...) noexcept(is_noexcept)>
{
    using type = return_t(args_t...) noexcept(is_noexcept);
};

template <typename return_t, typename T, bool is_noexcept, typename... args_t>
struct guide_helper<return_t (T::*)(args_t...) & noexcept(is_noexcept)>
{
    using type = return_t(args_t...) noexcept(is_noexcept);
};

template <typename return_t, typename T, bool is_noexcept, typename... args_t>
struct guide_helper<return_t (T::*)(args_t...) const noexcept(is_noexcept)>
{
    using type = return_t(args_t...) const noexcept(is_noexcept);
};

template <typename return_t, typename T, bool is_noexcept, typename... args_t>
struct guide_helper<return_t (T::*)(args_t...) const& noexcept(is_noexcept)>
{
    using type = return_t(args_t...) const noexcept(is_noexcept);
};

template <typename callable>
using guide_helper_t = typename guide_helper<decltype(&callable::operator())>::type;

} // namespace details::function_ns

template <
    typename callable,
    typename signature = details::function_ns::guide_helper_t<callable>,
    typename allocator = aligned_allocator<char>>
function(callable, allocator = allocator{}) -> function<signature, allocator>;

} // namespace q::util
