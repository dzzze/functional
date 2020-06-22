#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>

namespace dze::details::function_ns {

// This class is only available on little endian systems.
// Size must be greater than or equal to sizeof(alloc_details).
template <size_t Size, size_t Align, typename Alloc>
class storage : private Alloc
{
    struct alloc_details
    {
        void* data;
        size_t size;
        size_t alignment;
    };

public:
    using allocator_type = Alloc;
    using size_type = size_t;
    using const_pointer = const void*;
    using pointer = void*;

    explicit storage(const Alloc& alloc) noexcept
        : Alloc{alloc} {}

    storage(
        const size_type size, // NOLINT(readability-avoid-const-params-in-decls)
        size_type alignment,
        const Alloc& alloc = Alloc{}) noexcept(noexcept(this->allocate(size, alignment)))
        : Alloc{alloc}
    {
        if (size > Size || alignment > Align)
        {
            alignment = std::max(Align, alignment);
            const auto buf = allocate(size, alignment);
            init_alloc_details(buf, size, alignment);
        }
    }

    storage(storage&& other) noexcept
        : Alloc{std::move(other.get_allocator())}
        , m_storage{} {}

    ~storage() { deallocate(); }

    void move_allocator(storage& other)
    {
        get_allocator() = std::move(other.get_allocator());
    }

    void move_allocated(storage& other)
    {
        ::new (&as_alloc_details()) alloc_details{other.as_alloc_details()};
        m_storage.allocated = true;
        other.as_alloc_details().~alloc_details();
        other.m_storage.allocated = false;
    }

    void swap_allocator(storage& other) noexcept
    {
        swap(get_allocator(), other.get_allocator());
    }

    void swap_allocated(storage& other)
    {
        std::swap(as_alloc_details(), other.as_alloc_details());
    }

    // Discards the stored data if new_size results in allocation.
    void resize(const size_type size, size_t alignment)
        noexcept(noexcept(this->allocate(size, alignment)))
    {
        if (allocated())
        {
            if (size > allocated_size() || alignment > allocated_alignment())
            {
                alignment = std::max(Align, alignment);
                const auto buf = allocate(size, alignment);
                unchecked_deallocate();
                as_alloc_details() = {buf, size, alignment};
            }
        }
        else
        {
            if (size > Size || alignment > Align)
            {
                alignment = std::max(Align, alignment);
                const auto buf = allocate(size, alignment);
                init_alloc_details(buf, size, alignment);
            }
        }
    }

    void deallocate() noexcept
    {
        if (allocated())
            unchecked_deallocate();
    }

    [[nodiscard]] allocator_type get_allocator() const noexcept { return *this; }

    [[nodiscard]] bool allocated() const noexcept
    {
        return m_storage.allocated;
    }

    [[nodiscard]] const_pointer data() const noexcept
    {
        return allocated() ? allocated_data() : m_storage.data;
    }

    [[nodiscard]] pointer data() noexcept
    { 
        return allocated() ? allocated_data() : m_storage.data; 
    }

    [[nodiscard]] size_type allocated_size() const noexcept
    {
        return as_alloc_details().size;
    }

    [[nodiscard]] static constexpr size_t max_inline_size() noexcept
    {
        return Size;
    }

    [[nodiscard]] size_type allocated_alignment() const noexcept
    {
        return as_alloc_details().alignment;
    }

private:
    static_assert(
        Size >= sizeof(alloc_details),
        "Inline storage size of this object must be bigger than or equal to "
        "size of the dynamic allocation book keeping bits.");

    struct alignas(Align)
    {
        std::byte data[Size];
        bool allocated = false;
    } m_storage;

    [[nodiscard]] const alloc_details& as_alloc_details() const noexcept
    {
        return *reinterpret_cast<const alloc_details*>(&m_storage.data);
    }

    [[nodiscard]] alloc_details& as_alloc_details() noexcept
    {
        return *reinterpret_cast<alloc_details*>(&m_storage.data);
    }

    [[nodiscard]] const_pointer allocated_data() const noexcept { return as_alloc_details().data; }

    [[nodiscard]] pointer allocated_data() noexcept { return as_alloc_details().data; }

    void init_alloc_details(const pointer data, const size_t size, const size_t alignment) noexcept
    {
        ::new (&as_alloc_details()) alloc_details{data, size, alignment};
        m_storage.allocated = true;
    }

    [[nodiscard]] auto allocate(const size_t size, const size_t alignment)
        noexcept(noexcept(get_allocator().allocate_bytes(size, alignment)))
    {
        return get_allocator().allocate_bytes(size, alignment);
    }

    void unchecked_deallocate() noexcept
    {
        get_allocator().deallocate_bytes(
            allocated_data(), allocated_size(), allocated_alignment());
    }
};

} // namespace dze::details::function_ns
