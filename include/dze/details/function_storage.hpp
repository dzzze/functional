#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>

#include "../aligned_allocator.hpp" // TODO

namespace dze::details::function_ns {

// Defining this struct here to get its size for the default
// template parameter of the the following class.
struct non_sbo_t
{
    void* data;
    size_t size;
    size_t alignment;
};

// This class is only available on little endian systems.
// Size must be less than or equal to 2^(2 * CHAR_BIT - 1) and greater than or equal to
// sizeof(details::non_sbo_t).
template <
    size_t Size = sizeof(non_sbo_t),
    size_t Align = alignof(non_sbo_t),
    typename Alloc = aligned_allocator>
class storage : private Alloc
{
public:
    using allocator_type = Alloc;
    using size_type = size_t;
    using const_pointer = const void*;
    using pointer = void*;

    explicit storage(const Alloc& alloc) noexcept
        : Alloc{alloc} {}

    storage(
        const size_type size, // NOLINT(readability-avoid-const-params-in-decls)
        const size_type alignment, // NOLINT(readability-avoid-const-params-in-decls)
        const Alloc& alloc = Alloc{}) noexcept
        : Alloc{alloc}
    {
        if (size > Size || alignment > Align)
            init_non_sbo(size, std::max(Align, alignment));
    }

    storage(storage&& other) noexcept
        : Alloc{std::move(other.get_allocator())}
        , m_storage{} {}

    void move_allcator(storage& other)
    {
        get_allocator() = std::move(other.get_allocator());
    }

    void move_allocated(storage& other)
    {
        ::new (&as_non_sbo()) non_sbo_t{other.as_non_sbo()};
        m_storage.allocated = true;
        other.as_non_sbo().~non_sbo_t();
        other.m_storage.allocated = false;
    }

    ~storage()
    {
        if (allocated())
            deallocate();
    }

    // Undefined behavior if
    // std::allocator_traits<allocator_type>::propagate_on_container_swap::value
    // is false and get_allocator() != other.get_allocator().
    void swap_allocators(storage& other) noexcept
    {
        using alloc_traits = std::allocator_traits<Alloc>;

        if constexpr (alloc_traits::propagate_on_container_swap::value)
            swap(static_cast<Alloc&>(*this), static_cast<Alloc&>(other));
        else
            assert(get_allocator() == other.get_allocator());
    }

    // Discards the stored data if new_size results in allocation.
    void resize(const size_type size, size_t alignment) noexcept
    {
        if (allocated())
        {
            if (size > allocated_size() || alignment > allocated_alignment())
            {
                alignment = std::max(Align, alignment);
                const auto buf = allocate(size, alignment);
                deallocate();
                as_non_sbo() = {buf, size, alignment};
            }
        }
        else
        {
            if (size > Size || alignment > Align)
                init_non_sbo(size, std::max(Align, alignment));
        }
    }

    [[nodiscard]] allocator_type get_allocator() const { return *this; }

    [[nodiscard]] const_pointer data() const noexcept
    {
        return allocated() ? allocated_data() : m_storage.data;
    }

    [[nodiscard]] pointer data() noexcept
    { 
        return allocated() ? allocated_data() : m_storage.data; 
    }

    [[nodiscard]] bool allocated() const noexcept
    {
        return m_storage.allocated;
    }

private:
    static_assert(
        Size >= sizeof(non_sbo_t),
        "Inline storage size of this object must be bigger than or equal to "
        "size of the dynamic allocation book keeping bits.");

    struct alignas(Align)
    {
        std::byte data[Size];
        bool allocated = false;
    } m_storage;

    [[nodiscard]] const non_sbo_t& as_non_sbo() const noexcept
    {
        return *reinterpret_cast<const non_sbo_t*>(&m_storage.data);
    }

    [[nodiscard]] non_sbo_t& as_non_sbo() noexcept
    {
        return *reinterpret_cast<non_sbo_t*>(&m_storage.data);
    }

    [[nodiscard]] const_pointer allocated_data() const noexcept { return as_non_sbo().data; }

    [[nodiscard]] pointer allocated_data() noexcept { return as_non_sbo().data; }

    [[nodiscard]] size_type allocated_size() const noexcept
    {
        return as_non_sbo().size;
    }

    [[nodiscard]] size_type allocated_alignment() const noexcept
    {
        return as_non_sbo().alignment;
    }

    void init_non_sbo(const size_t size, const size_t alignment) noexcept
    {
        init_non_sbo(allocate(size, alignment), size, alignment);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    void init_non_sbo(const pointer data, const size_t size, const size_t alignment) noexcept
    {
        ::new (&as_non_sbo()) non_sbo_t{data, size, alignment};
        m_storage.allocated = true;
    }

    [[nodiscard]] auto allocate(const size_t size, const size_t alignment) noexcept
    {
        return get_allocator().allocate_bytes(size, alignment);
    }

    void deallocate() noexcept
    {
        get_allocator().deallocate_bytes(allocated_data(), allocated_alignment());
    }
};

} // namespace dze::details::function_ns
