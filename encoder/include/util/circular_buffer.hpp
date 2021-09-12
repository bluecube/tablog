#pragma once

#include <cstddef>
#include <array>

namespace tablog::util {

/// Fixed size circular buffer.
/// Never allocates, never throws.
/// Indexing type may be replaced for MCU compatibility.
template <typename T, std::size_t N, typename IndexT=std::size_t>
class CircularBuffer {
public:
    using value_type = T;

    /// Push an item to the buffer, removes the oldest one if buffer is full
    void push_back(const T& v) noexcept {
        if (used < N) {
            a[used++] = v;
        }
        else
        {
            a[startOffset] = v;
            startOffset = index(1);
        }

    }

    /// Remove the last item in the buffer, undefined behavior if the
    /// buffer is empty.
    value_type pop_front() noexcept {
        used--;
        auto idx = startOffset;
        startOffset = index(1);
        return a[idx];
    }

    bool empty() const noexcept {
        return used == 0;
    }

    /// Number of items currently held in the buffer
    IndexT size() const noexcept {
        return used;
    }

    /// Maximum number of items held in this buffer.
    static IndexT capacity() noexcept {
        return N;
    }

    /// Access i-th element.
    /// Accessing nonexistent item is UB.
    const value_type& operator[](IndexT i) const noexcept
    {
        return a[index(i)];
    }

    /// Access i-th element.
    /// Accessing nonexistent item is UB.
    value_type& operator[](IndexT i) noexcept
    {
        return a[index(i)];
    }

    /// Access first element.
    /// It is UB if the buffer is empty.
    value_type& front() noexcept {
        return a[startOffset];
    }

    /// Access first element.
    /// It is UB if the buffer is empty.
    const value_type& front() const noexcept {
        return a[startOffset];
    }

    /// Access last element.
    /// It is UB if the buffer is empty.
    value_type& back() noexcept {
        return a[index(used - 1)];
    }

    /// Access last element.
    /// It is UB if the buffer is empty.
    const value_type& back() const noexcept {
        return a[index(used - 1)];
    }

private:
    IndexT index(IndexT i) const noexcept {
        return (startOffset + i) % N;
    }

    IndexT startOffset = 0;
    IndexT used = 0;
    T a[N];
};

}