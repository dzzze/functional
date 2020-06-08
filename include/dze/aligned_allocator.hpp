#pragma once

#include <cstddef>
#include <new>
#include <type_traits>

namespace dze {

template <typename T = std::byte>
class aligned_allocator
{
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    aligned_allocator() = default;

    constexpr aligned_allocator(const aligned_allocator&) noexcept = default;

    template <typename U>
    constexpr aligned_allocator(const aligned_allocator<U>&) noexcept {}

    constexpr aligned_allocator& operator=(const aligned_allocator&) noexcept = default;

    template <typename U>
    constexpr aligned_allocator& operator=(const aligned_allocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(const size_t n) const noexcept
    {
        return allocate(n, alignof(T));
    }

    [[nodiscard]] T* allocate(const size_t n, const size_t alignment) const noexcept
    {
        return static_cast<T*>(::operator new(
            n * sizeof(T), std::align_val_t{alignment}, std::nothrow_t{}));
    }

    void deallocate(T* const p, const size_t alignment) const noexcept
    {
        ::operator delete(p, std::align_val_t{alignment});
    }
};

template <typename T1, typename T2>
[[nodiscard]] constexpr bool operator==(
    const aligned_allocator<T1>, const aligned_allocator<T2>) noexcept
{
    return true;
}

template <typename T1, typename T2>
[[nodiscard]] constexpr bool operator!=(
    const aligned_allocator<T1>, const aligned_allocator<T2>) noexcept
{
    return false;
}

} // namespace q::utll
