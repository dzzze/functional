#pragma once

#include <cstddef>
#include <new>
#include <type_traits>

namespace dze {

class aligned_allocator
{
public:
    using value_type = std::byte;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] void* allocate_bytes(const size_t n, const size_t alignment) const noexcept
    {
        return ::operator new(n, std::align_val_t{alignment}, std::nothrow_t{});
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void deallocate_bytes(void* const p, const size_t alignment) const noexcept
    {
        ::operator delete(p, std::align_val_t{alignment});
    }
};

[[nodiscard]] constexpr bool operator==(aligned_allocator, aligned_allocator) noexcept
{
    return true;
}

[[nodiscard]] constexpr bool operator!=(aligned_allocator, aligned_allocator) noexcept
{
    return false;
}

} // namespace q::utll
