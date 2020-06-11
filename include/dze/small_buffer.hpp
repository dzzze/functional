#pragma once

#include <cassert>
#include <climits>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

#include <dze/type_traits.hpp>

#include "aligned_allocator.hpp"

namespace dze {

namespace details::small_buffer_ns {

// Defining this struct here to get its size for the default
// template parameter of the the following class.
struct non_sbo_t
{
    using value_type = std::byte;
    using size_type = size_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    pointer data;
    size_type size;
    size_type alignment;
    size_type capacity;
};

template <size_t Size, bool = Size == sizeof(non_sbo_t)>
struct storage_t
{
    non_sbo_t non_sbo;
};

template <size_t Size>
struct storage_t<Size, false>
{
    std::byte padding[Size - sizeof(non_sbo_t)];
    non_sbo_t non_sbo;
};

} // namespace details::small_buffer_ns

// This class is only available on little endian systems.
// Size must be a multiple of the sizeof(void*) and must be less
// than or equal to 2^(CHAR_BIT - 1) and greater than or equal to 3 *
// sizeof(void*).
template <
    size_t Size = sizeof(details::small_buffer_ns::non_sbo_t),
    size_t Align = alignof(details::small_buffer_ns::non_sbo_t),
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
class small_buffer
    : private Alloc
    , private enable_copy_move<
        true,
        std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value,
        true,
        std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value>
{
    // Inheriting from the Alloc to benefit from empty base
    // optimization.
    // TODO: C++20 use [[no_unique_address]] for the same effect.

    using non_sbo_t = details::small_buffer_ns::non_sbo_t;
    using alloc_traits = std::allocator_traits<Alloc>;

public:
    using value_type = non_sbo_t::value_type;
    using allocator_type = Alloc;
    using size_type = non_sbo_t::size_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = non_sbo_t::pointer;
    using const_pointer = non_sbo_t::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;

    small_buffer() noexcept(noexcept(Alloc{}))
        : small_buffer{Alloc{}} {}

    explicit small_buffer(const Alloc& alloc) noexcept
        : Alloc{alloc}
    {
        set_sbo_size(0);
    }

    // NOLINTNEXTLINE(readability-avoid-const-params-in-decls)
    explicit small_buffer(const size_type size, const Alloc& alloc = Alloc{}) noexcept
        : Alloc{alloc}
    {
        if (size > max_sbo_capacity())
            init_non_sbo(size, Align);
        else
            set_sbo_size(size);
    }

    small_buffer(
        const size_type size, // NOLINT(readability-avoid-const-params-in-decls)
        const size_type alignment, // NOLINT(readability-avoid-const-params-in-decls)
        const Alloc& alloc = Alloc{}) noexcept
        : Alloc{alloc}
    {
        if (size > max_sbo_capacity() || alignment > Align)
            init_non_sbo(size, alignment);
        else
            set_sbo_size(size);
    }

    small_buffer(
        const size_type size, // NOLINT(readability-avoid-const-params-in-decls)
        const value_type value, // NOLINT(readability-avoid-const-params-in-decls)
        const Alloc& alloc = Alloc{}) noexcept
        : small_buffer{size, alloc}
    {
        std::uninitialized_fill(begin(), end(), value);
    }

    small_buffer(
        const size_type size, // NOLINT(readability-avoid-const-params-in-decls)
        const size_type alignment, // NOLINT(readability-avoid-const-params-in-decls)
        const value_type value, // NOLINT(readability-avoid-const-params-in-decls)
        const Alloc& alloc = Alloc{}) noexcept
        : small_buffer{size, alignment, alloc}
    {
        std::uninitialized_fill(begin(), end(), value);
    }

    small_buffer(const small_buffer& other) noexcept
        : small_buffer{
            other,
            alloc_traits::select_on_container_copy_construction(other.get_allocator())} {}

    small_buffer(const small_buffer& other, const Alloc& alloc) noexcept : Alloc{alloc}
    {
        if (other.sbo())
            copy_sbo(other);
        else
        {
            init_non_sbo(other.non_sbo_size(), other.non_sbo_alignment());
            copy_non_sbo(other);
        }
    }

    // Copy assignment is only enabled when this object can always
    // reuse its allocated memory.
    small_buffer& operator=(const small_buffer& other) noexcept
    {
        if (sbo() && other.sbo())
            copy_sbo(other);
        else if (other.sbo())
            copy_sbo_to_non_sbo(other);
        else if (sbo())
        {
            const auto size = other.non_sbo_size();
            const auto alignment = other.non_sbo_alignment();
            if (size > max_sbo_capacity() || alignment > Align)
            {
                if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
                    static_cast<Alloc&>(*this) = static_cast<Alloc&>(other);
                init_non_sbo(size, alignment);
                copy_non_sbo(other);
            }
            else
                copy_non_sbo_to_sbo(other);
        }
        else
        {
            const auto size = other.non_sbo_size();
            const auto alignment = other.non_sbo_alignment();
            if (size > non_sbo_capacity() || alignment > non_sbo_alignment())
            {
                deallocate();
                if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
                    static_cast<Alloc&>(*this) = static_cast<Alloc&>(other);
                init_non_sbo(size, alignment);
            }
            else
                set_non_sbo_size(size);
            copy_non_sbo(other);
        }
        return *this;
    }

    small_buffer(small_buffer&& other) noexcept
        : Alloc{std::move(static_cast<Alloc&>(other))}
    {
        if (other.sbo())
            copy_sbo(other);
        else
        {
            as_non_sbo() = other.as_non_sbo();
            other.set_sbo_size(0);
        }
    }

    // Move assignment is only enabled when this object can always
    // take ownership of the source memory.
    small_buffer& operator=(small_buffer&& other) noexcept
    {
        if (sbo() && other.sbo())
            copy_sbo(other);
        else if (other.sbo())
            copy_sbo_to_non_sbo(other);
        else
        {
            if (!sbo())
                deallocate();
            if constexpr (alloc_traits::propagate_on_container_move_assignment::value)
                static_cast<Alloc&>(*this) = static_cast<Alloc&>(other);
            as_non_sbo() = other.as_non_sbo();
            other.set_sbo_size(0);
        }
        return *this;
    }

    ~small_buffer()
    {
        if (!sbo())
            deallocate();
    }

    // Undefined behavior if
    // std::allocator_traits<allocator_type>::propagate_on_container_swap::value
    // is false and get_allocator() != other.get_allocator().
    void swap(small_buffer& other) noexcept
    {
        if constexpr (alloc_traits::propagate_on_container_swap::value)
            swap(static_cast<Alloc&>(*this), static_cast<Alloc&>(other));
        else
            assert(get_allocator() == other.get_allocator());

        std::swap(m_storage, other.m_storage);
    }

    void resize(const size_type new_size) noexcept
    {
        resize(new_size, Align);
    }

    void resize(const size_type new_size, const size_type alignment) noexcept
    {
        if (sbo())
        {
            if (new_size > max_sbo_capacity() || alignment > Align)
                resize_convert_to_non_sbo(new_size, alignment);
            else
                set_sbo_size(new_size);
        }
        else
        {
            if (new_size > non_sbo_capacity() || alignment > non_sbo_alignment())
                resize_expand_non_sbo(new_size, alignment);
            else
                set_non_sbo_size(new_size);
        }
    }

    void resize(const size_type new_size, const value_type value) noexcept
    {
        const auto old_size = size();
        resize(new_size);
        if (old_size < new_size)
            std::uninitialized_fill(data() + old_size, data() + new_size, value);
    }

    // Discards the stored data if new_size results in allocation.
    void resize_discard(const size_type new_size) noexcept
    {
        resize_discard(new_size, Align);
    }

    // Discards the stored data if new_size results in allocation.
    void resize_discard(const size_type new_size, const size_t alignment) noexcept
    {
        if (sbo())
        {
            if (new_size > max_sbo_capacity() || alignment > Align)
                init_non_sbo(new_size, alignment);
            else
                set_sbo_size(new_size);
        }
        else
        {
            if (new_size > non_sbo_capacity())
                resize_discard_expand_non_sbo(new_size, alignment);
            else
                set_non_sbo_size(new_size);
        }
    }

    void reserve(const size_type new_cap) noexcept
    {
        if (sbo())
        {
            if (new_cap > max_sbo_capacity())
            {
                auto new_storage = allocate(new_cap, Align);
                relocate(new_storage);
                init_non_sbo(new_storage, sbo_size(), new_cap);
                set_non_sbo_alignment(Align);
            }
        }
        else
        {
            if (new_cap > non_sbo_capacity())
            {
                auto new_storage = allocate(new_cap, non_sbo_alignment());
                relocate(new_storage);
                deallocate();
                as_non_sbo().data = new_storage;
                set_non_sbo_capacity(new_cap);
            }
        }
    }

    // Discards the stored data if new_cap results in allocation.
    void reserve_discard(const size_type new_cap) noexcept
    {
        if (sbo())
        {
            if (new_cap > max_sbo_capacity())
            {
                auto new_storage = allocate(new_cap, Align);
                init_non_sbo(new_storage, sbo_size(), new_cap);
                set_non_sbo_alignment(Align);
            }
        }
        else
        {
            if (new_cap > non_sbo_capacity())
            {
                auto new_storage = allocate(new_cap, non_sbo_alignment());
                deallocate();
                as_non_sbo().data = new_storage;
                set_non_sbo_capacity(new_cap);
            }
        }
    }

    [[nodiscard]] allocator_type get_allocator() const { return *this; }

    [[nodiscard]] size_type size() const noexcept
    {
        return sbo() ? sbo_size() : non_sbo_size();
    }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    [[nodiscard]] static constexpr size_type max_sbo_capacity() noexcept
    {
        return Size - 1;
    }

    [[nodiscard]] size_type capacity() const noexcept
    {
        return sbo() ? max_sbo_capacity() : non_sbo_capacity();
    }

    [[nodiscard]] const_reference operator[](const size_type pos) const noexcept
    {
        assert(pos < size());

        return data()[pos];
    }

    [[nodiscard]] reference operator[](const size_type pos) noexcept
    {
        assert(pos < size());

        return data()[pos];
    }

    [[nodiscard]] const_pointer data() const noexcept
    {
        return sbo() ? sbo_data() : non_sbo_data();
    }

    [[nodiscard]] pointer data() noexcept { return sbo() ? sbo_data() : non_sbo_data(); }

    [[nodiscard]] const_iterator begin() const noexcept { return data(); }

    [[nodiscard]] iterator begin() noexcept { return data(); }

    [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return sbo() ? sbo_data() + sbo_size() : non_sbo_data() + non_sbo_size();
    }

    [[nodiscard]] iterator end() noexcept
    {
        return sbo() ? sbo_data() + sbo_size() : non_sbo_data() + non_sbo_size();
    }

    [[nodiscard]] const_iterator cend() const noexcept { return end(); }

private:
    static_assert(
        Size >= sizeof(non_sbo_t),
        "Stack size of this object must be bigger than or equal to "
        "size of the dynamic allocation book keeping bits.");

    static_assert(
        Size <= 1 << (CHAR_BIT - 1),
        "Stack size of this object must be less than or equal to "
        "the maximum value addressable with CHAR_BIT - 1.");

    static_assert(
        Size % alignof(non_sbo_t) == 0,
        "Size of this object must be a multiple of the alignment "
        "of void* in this platform.");

    static_assert(
        Align % alignof(non_sbo_t) == 0,
        "Alignment of this object must be a multiple of the alignment "
        "of void* in this platform.");

    static constexpr std::byte short_mask{0x1 << (CHAR_BIT - 1)};
    static constexpr size_type long_mask =
        size_type{0x1} <<
            (std::numeric_limits<size_type>::digits - 1);

    alignas(Align) details::small_buffer_ns::storage_t<Size> m_storage;

    [[nodiscard]] bool sbo() const noexcept
    {
        return
            (static_cast<std::byte>(sbo_data()[Size - 1]) & short_mask) ==
            static_cast<std::byte>(0);
    }

    [[nodiscard]] const non_sbo_t& as_non_sbo() const noexcept { return m_storage.non_sbo; }

    [[nodiscard]] non_sbo_t& as_non_sbo() noexcept { return m_storage.non_sbo; }

    [[nodiscard]] const_pointer sbo_data() const noexcept
    {
        return reinterpret_cast<const_pointer>(&m_storage);
    }

    [[nodiscard]] pointer sbo_data() noexcept
    {
        return reinterpret_cast<pointer>(&m_storage);
    }

    [[nodiscard]] size_type sbo_size() const noexcept
    {
        return std::to_integer<size_type>(sbo_data()[Size - 1]);
    }

    void set_sbo_size(const size_type size) noexcept
    {
        assert(size <= max_sbo_capacity());

        sbo_data()[Size - 1] = static_cast<std::byte>(size);
    }

    [[nodiscard]] const_pointer non_sbo_data() const noexcept { return as_non_sbo().data; }

    [[nodiscard]] pointer non_sbo_data() noexcept { return as_non_sbo().data; }

    [[nodiscard]] size_type non_sbo_size() const noexcept
    {
        return as_non_sbo().size;
    }

    void set_non_sbo_size(const size_type size) noexcept
    {
        as_non_sbo().size = size;
    }

    [[nodiscard]] size_type non_sbo_alignment() const noexcept
    {
        return as_non_sbo().alignment;
    }

    void set_non_sbo_alignment(const size_type alignment) noexcept
    {
        as_non_sbo().alignment = alignment;
    }

    [[nodiscard]] size_type non_sbo_capacity() const noexcept
    {
        return as_non_sbo().capacity & ~long_mask;
    }

    void set_non_sbo_capacity(const size_type cap) noexcept
    {
        assert(
            cap <
            std::numeric_limits<size_type>::max() >> std::numeric_limits<value_type>::digits);

        as_non_sbo().capacity = cap | long_mask;
    }

    void init_non_sbo(const size_t cap, const size_t alignment) noexcept
    {
        set_non_sbo_alignment(alignment);
        init_non_sbo(allocate(cap, alignment), cap);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    void init_non_sbo(const pointer data, const size_t cap) noexcept
    {
        init_non_sbo(data, cap, cap);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    void init_non_sbo(const pointer data, const size_t size, const size_t cap) noexcept
    {
        as_non_sbo().data = data;
        set_non_sbo_size(size);
        set_non_sbo_capacity(cap);
    }

    void copy_sbo(const small_buffer& other) noexcept
    {
        const auto size = other.sbo_size();
        set_sbo_size(size);
        std::uninitialized_copy(other.sbo_data(), other.sbo_data() + size, sbo_data());
    }

    void copy_sbo_to_non_sbo(const small_buffer& other) noexcept
    {
        const auto size = other.sbo_size();
        set_non_sbo_size(size);
        std::uninitialized_copy(other.sbo_data(), other.sbo_data() + size, non_sbo_data());
    }

    void copy_non_sbo(const small_buffer& other) noexcept
    {
        // This happens after full reset of the non_sbo object so
        // setting the non_sbo size is not necessary.
        std::uninitialized_copy(
            other.non_sbo_data(), other.non_sbo_data() + non_sbo_size(), non_sbo_data());
    }

    void copy_non_sbo_to_sbo(const small_buffer& other) noexcept
    {
        const auto size = other.non_sbo_size();
        set_sbo_size(size);
        std::uninitialized_copy(other.non_sbo_data(), other.non_sbo_data() + size, sbo_data());
    }

    [[nodiscard]] auto allocate(const size_t size, const size_t alignment) noexcept
    {
        return get_allocator().allocate(size, alignment);
    }

    void deallocate() noexcept
    {
        get_allocator().deallocate(non_sbo_data(), non_sbo_alignment());
    }

    void relocate(const pointer destination) const noexcept
    {
        std::uninitialized_copy(begin(), end(), destination);
    }

    void resize_convert_to_non_sbo(const size_type new_size, const size_t alignment) noexcept
    {
        auto new_storage = allocate(new_size, alignment);
        relocate(new_storage);
        init_non_sbo(new_storage, new_size);
        set_non_sbo_alignment(alignment);
    }

    void resize_expand_non_sbo(const size_type new_size, const size_t alignment) noexcept
    {
        auto new_storage = allocate(new_size, alignment);
        relocate(new_storage);
        deallocate();
        init_non_sbo(new_storage, new_size);
        set_non_sbo_alignment(alignment);
    }

    void resize_discard_expand_non_sbo(
        const size_type new_size, const size_t alignment) noexcept
    {
        auto new_storage = allocate(new_size, alignment);
        deallocate();
        init_non_sbo(new_storage, new_size);
        set_non_sbo_alignment(alignment);
    }
};

template <
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
small_buffer(Alloc = Alloc{}) -> small_buffer<
    sizeof(details::small_buffer_ns::non_sbo_t),
    alignof(details::small_buffer_ns::non_sbo_t),
    Alloc>;

template <
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
small_buffer(size_t, Alloc = Alloc{}) -> small_buffer<
    sizeof(details::small_buffer_ns::non_sbo_t),
    alignof(details::small_buffer_ns::non_sbo_t),
    Alloc>;

template <
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
small_buffer(size_t, size_t, Alloc = Alloc{}) -> small_buffer<
    sizeof(details::small_buffer_ns::non_sbo_t),
    alignof(details::small_buffer_ns::non_sbo_t),
    Alloc>;

template <
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
small_buffer(size_t, std::byte, Alloc = Alloc{}) -> small_buffer<
    sizeof(details::small_buffer_ns::non_sbo_t),
    alignof(details::small_buffer_ns::non_sbo_t),
    Alloc>;

template <
    typename Alloc = aligned_allocator<details::small_buffer_ns::non_sbo_t::value_type>>
small_buffer(size_t, size_t, std::byte, Alloc = Alloc{}) -> small_buffer<
    sizeof(details::small_buffer_ns::non_sbo_t),
    alignof(details::small_buffer_ns::non_sbo_t),
    Alloc>;

} // namespace dze
