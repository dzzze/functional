#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include <dze/allocator.hpp>
#include <dze/memory_resource.hpp>
#include <dze/type_traits.hpp>

#include "details/function/delegate.hpp"
#include "details/function/storage.hpp"

namespace dze {

namespace details::function_ns {

template <typename From, typename To>
inline constexpr bool is_safely_convertible_v =
    !std::is_reference_v<To> || std::is_reference_v<From>;

template <typename, typename>
class base;

template <typename Function, bool Noexcept, typename R, typename... Args>
class base<Function, R(Args...) noexcept(Noexcept)>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args) noexcept(Noexcept)
    {
        auto& obj = *static_cast<Function*>(this);
        return obj.m_delegate.call(obj.data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            (Noexcept
                ? std::is_nothrow_invocable_v<std::decay_t<Callable>&, Args...>
                : std::is_invocable_v<std::decay_t<Callable>&, Args...>) &&
            is_safely_convertible_v<
                std::invoke_result_t<std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), Noexcept>;
    using const_signature = R(Args...) const noexcept(Noexcept);
    using mut_signature = R(Args...) noexcept(Noexcept);

    static constexpr bool is_const = false;

    template <typename Callable>
    static constexpr bool is_convertible_v = is_convertible<Callable>::value;
};

template <typename Function, bool Noexcept, typename R, typename... Args>
class base<Function, R(Args...) const noexcept(Noexcept)>
{
public:
    // Pre-condition: A call is stored in this object.
    R operator()(Args... args) const noexcept(Noexcept)
    {
        auto& obj = *static_cast<const Function*>(this);
        return obj.m_delegate.call(obj.data_addr(), static_cast<Args&&>(args)...);
    }

private:
    template <typename Callable, typename = void>
    struct is_convertible : std::false_type {};

    template <typename Callable>
    struct is_convertible<
        Callable,
        std::enable_if_t<
            (Noexcept
                ? std::is_nothrow_invocable_v<const std::decay_t<Callable>&, Args...>
                : std::is_invocable_v<const std::decay_t<Callable>&, Args...>) &&
            is_safely_convertible_v<
                std::invoke_result_t<const std::decay_t<Callable>&, Args...>, R>>>
        : std::true_type {};

protected:
    using delegate_type = delegate_t<R(Args...), Noexcept>;
    using const_signature = R(Args...) const noexcept(Noexcept);
    using mut_signature = R(Args...) noexcept(Noexcept);

    static constexpr bool is_const = true;

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
template <typename Signature, typename Alloc = allocator>
class function : public details::function_ns::base<function<Signature, Alloc>, Signature>
{
    using base = details::function_ns::base<function, Signature>;

    struct conv_tag_t {};

    template <typename Sig>
    static constexpr bool is_movable_v =
        std::is_same_v<Sig, Signature> ||
        std::is_same_v<Sig, typename base::const_signature>;

public:
    using allocator_type = Alloc;

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
    function(Callable call, const Alloc& alloc = Alloc{})
        noexcept(std::is_nothrow_constructible_v<
            decltype(this->m_storage), size_t, size_t, const Alloc&>)
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
        : m_storage{std::move(other.m_storage)}
        , m_delegate{other.m_delegate}
    {
        if (other.m_storage.allocated())
            m_storage.move_allocated(other.m_storage);
        else
        {
            other.m_delegate.move(other.data_addr(), data_addr());
            other.m_delegate.destroy(other.data_addr());
        }
        other.m_delegate.reset();
    }

    template <
        typename Signature2 = Signature,
        typename Alloc2 = Alloc,
        DZE_REQUIRES(
            !is_movable_v<Signature2> &&
            base::template is_convertible_v<function<Signature2>>)>
    function(function<Signature2, Alloc2>&& other, const Alloc& alloc = Alloc{})
        noexcept(
            std::is_nothrow_constructible_v<function, decltype(other), const Alloc&, conv_tag_t>)
        : function{std::move(other), alloc, conv_tag_t{}} {}

    template <typename Signature2 = Signature,
        DZE_REQUIRES(is_movable_v<Signature2>)>
    function& operator=(function<Signature2, Alloc>&& other)
        noexcept(noexcept(this->m_storage.resize(0, 0)))
    {
        using alloc_traits = std::allocator_traits<Alloc>;

        m_delegate.destroy(data_addr());
        m_delegate = other.m_delegate;
        if (other.m_storage.allocated())
        {
            if constexpr (alloc_traits::propagate_on_container_move_assignment::value)
            {
                m_storage.deallocate();
                m_storage.move_allocator(other.m_storage);
                m_storage.move_allocated(other.m_storage);
            }
            else if constexpr (alloc_traits::is_always_equal::value)
            {
                m_storage.deallocate();
                m_storage.move_allocated(other.m_storage);
            }
            else
            {
                if (m_storage.get_allocator() == other.m_storage.get_allocator())
                {
                    m_storage.deallocate();
                    m_storage.move_allocated(other.m_storage);
                }
                else
                {
                    m_storage.resize(
                        other.m_storage.allocated_size(), other.m_storage.allocated_alignment());
                    other.m_delegate.move(other.data_addr(), data_addr());
                    other.m_delegate.destroy(other.data_addr());
                }
            }
        }
        else
        {
            other.m_delegate.move(other.data_addr(), data_addr());
            other.m_delegate.destroy(other.data_addr());
        }
        other.m_delegate.reset();
        return *this;
    }

    template <typename Signature2 = Signature, typename Alloc2 = Alloc,
        DZE_REQUIRES(
            !is_movable_v<Signature2> &&
            base::template is_convertible_v<function<Signature2>>)>
    function& operator=(function<Signature2, Alloc2>&& other)
        noexcept(noexcept(this->assign(std::move(other))))
    {
        assign(std::move(other));
        return *this;
    }

    ~function() { m_delegate.destroy(data_addr()); }

    // Undefined behavior if std::allocator_traits<Alloc>::propagate_on_container_swap == false
    // and both allocators do not compare equal.
    void swap(function& other) noexcept(noexcept(this->m_storage.resize(0, 0)))
    {
        using alloc_traits = std::allocator_traits<Alloc>;

        std::swap(m_delegate, other.m_delegate);
        alignas(std::max_align_t) std::byte temp[decltype(m_storage)::max_inline_size()];
        if (other.m_storage.allocated())
        {
            if (m_storage.allocated())
            {
                if constexpr (alloc_traits::propagate_on_container_swap::value)
                {
                    m_storage.swap_allocator(other.m_storage);
                    m_storage.swap_allocated(other.m_storage);
                }
                else if constexpr (alloc_traits::is_always_equal::value)
                    m_storage.swap_allocated(other.m_storage);
                else
                {
                    if (m_storage.get_allocator() == other.m_storage.get_allocator())
                        m_storage.swap_allocated(other.m_storage);
                    else
                        assert(false);
                }
            }
            else
            {
                other.m_delegate.move(data_addr(), temp);
                swap_helper(*this, other);
                other.m_delegate.move(temp, other.data_addr());
            }
        }
        else
        {
            m_delegate.move(other.data_addr(), temp);
            if (m_storage.allocated())
                swap_helper(other, *this);
            else
                other.m_delegate.move(data_addr(), other.data_addr());
            m_delegate.move(temp, data_addr());
        }
    }

    function& operator=(std::nullptr_t) noexcept
    {
        m_delegate.destroy(data_addr());
        m_delegate.reset();
        return *this;
    }

    template <typename Callable,
        DZE_REQUIRES(!is_function_v<Callable> && base::template is_convertible_v<Callable>)>
    function& operator=(Callable call) noexcept(noexcept(this->assign(std::move(call))))
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

    // Deallcates storage if there is no callable object stored.
    void shrink_to_fit() noexcept
    {
        if (!*this)
            m_storage.deallocate();
    }

private:
    using delegate_type = typename base::delegate_type;

    friend base;
    friend class function<typename base::mut_signature, Alloc>;

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

    details::function_ns::storage<
        80 - 1 - sizeof(delegate_type), alignof(std::max_align_t), Alloc> m_storage;
    delegate_type m_delegate;

    template <typename Callable>
    function(Callable&& call, const Alloc& alloc, conv_tag_t)
        noexcept(std::is_nothrow_constructible_v<
            decltype(this->m_storage), size_t, size_t, const Alloc&>)
        : m_storage{sizeof(std::decay_t<Callable>), alignof(std::decay_t<Callable>), alloc}
    {
        m_delegate.template set<Callable, base::is_const>();
        ::new (data_addr()) std::decay_t<Callable>{std::move(call)};
    }

    template <typename Callable>
    void assign(Callable&& call)
        noexcept(noexcept(
            m_storage.resize(sizeof(std::decay_t<Callable>), alignof(std::decay_t<Callable>))))
    {
        m_delegate.destroy(data_addr());
        m_delegate.template set<Callable, base::is_const>();
        m_storage.resize(sizeof(std::decay_t<Callable>), alignof(std::decay_t<Callable>));
        ::new (data_addr()) std::decay_t<Callable>{std::move(call)};
    }

    [[nodiscard]] const void* data_addr() const noexcept { return m_storage.data(); }

    [[nodiscard]] void* data_addr() noexcept { return m_storage.data(); }

    static void swap_helper(function& lhs, function& rhs)
        noexcept(noexcept(lhs.m_storage.resize(0, 0)))
    {
        using alloc_traits = std::allocator_traits<Alloc>;

        if constexpr (alloc_traits::propagate_on_container_move_assignment::value)
        {
            lhs.m_storage.move_allocator(rhs.m_storage);
            lhs.m_storage.move_allocated(rhs.m_storage);
        }
        else if constexpr (alloc_traits::is_always_equal::value)
            lhs.m_storage.move_allocated(rhs.m_storage);
        else
        {
            if (lhs.m_storage.get_allocator() == rhs.m_storage.get_allocator())
                lhs.m_storage.move_allocated(rhs.m_storage);
            else
            {
                lhs.m_storage.resize(
                    rhs.m_storage.allocated_size(), rhs.m_storage.allocated_alignment());
                lhs.m_delegate.move(rhs.data_addr(), lhs.data_addr());
            }
        }
    }
};

template <typename R, typename... Args, typename Alloc = allocator>
function(R(*)(Args...), Alloc = Alloc{}) -> function<R(Args...) const, Alloc>;

template <typename R, typename... Args, typename Alloc = allocator>
function(R(*)(Args...) noexcept, Alloc = Alloc{}) ->
    function<R(Args...) const noexcept, Alloc>;

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
    typename Alloc = allocator>
function(Callable, Alloc = Alloc{}) -> function<Signature, Alloc>;

namespace pmr {

template <typename Signature>
using function = ::dze::function<Signature, polymorphic_allocator>;

} // namespace pmr

} // namespace dze
