#ifndef __EZ_STACK_ALLOCATOR_HPP__
#define __EZ_STACK_ALLOCATOR_HPP__


#include <cstddef>
#include <cassert>
#include <utility>

namespace ez {

template <std::size_t N, std::size_t alignment = alignof(std::max_align_t)>
class StackArea
{
    alignas(alignment) char _buf[N];
    char* _ptr;

public:
    ~StackArea() { _ptr = nullptr; }
    StackArea() noexcept : _ptr(_buf) {}
    StackArea(const StackArea&) = delete;
    StackArea& operator=(const StackArea&) = delete;

    template <std::size_t ReqAlign> char* allocate(std::size_t n);
    void deallocate(char* p, std::size_t n) noexcept;

    static constexpr std::size_t size() noexcept { return N; }
    std::size_t used() const noexcept { return static_cast<std::size_t>(_ptr - _buf); }
    void reset() noexcept { _ptr = _buf; }

private:
    static
    std::size_t align_up(std::size_t n) noexcept { return (n + (alignment-1)) & ~(alignment-1); }

    bool pointer_in_buffer(char* p) noexcept { return _buf <= p && p <= _buf + N; }
};

template <std::size_t N, std::size_t alignment>
template <std::size_t ReqAlign>
char*
StackArea<N, alignment>::allocate(std::size_t n)
{
    static_assert(ReqAlign <= alignment, "alignment is too small for this area");
    assert(pointer_in_buffer(_ptr) && "StackAllocator has outlived area");
    
    auto const aligned_n = align_up(n);
    if (static_cast<decltype(aligned_n)>(_buf + N - _ptr) >= aligned_n)
    {
        char* r = _ptr;
        _ptr += aligned_n;
        return r;
    }

    static_assert(alignment <= alignof(std::max_align_t), "you've chosen an "
                  "alignment that is larger than alignof(std::max_align_t), and "
                  "cannot be guaranteed by normal operator new");
    return static_cast<char*>(::operator new(n));
}

template <std::size_t N, std::size_t alignment>
void
StackArea<N, alignment>::deallocate(char* p, std::size_t n) noexcept
{
    assert(pointer_in_buffer(_ptr) && "StackAllocator has outlived area");
    
    if (pointer_in_buffer(p))
    {
        n = align_up(n);
        if (p + n == _ptr)
            _ptr = p;
    }
    else
        ::operator delete(p);
}

template <class T, std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class StackAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    static auto constexpr alignment = Align;
    static auto constexpr size = N;
    using area_type = StackArea<size, alignment>;

private:
    area_type& _area;

public:
    StackAllocator(const StackAllocator&) = default;
    StackAllocator& operator=(const StackAllocator&) = delete;

    StackAllocator(area_type& a) noexcept : _area(a)
    {
        static_assert(size % alignment == 0,
                      "size N needs to be a multiple of alignment Align");
    }
    template <class U>
    StackAllocator(const StackAllocator<U, N, alignment>& area) noexcept
        : _area(area._area) {}

    template <class _Up> struct rebind {using other = StackAllocator<_Up, N, alignment>;};

    T* allocate(std::size_t n)
    {
        return reinterpret_cast<T*>(_area. template allocate<alignof(T)>(n*sizeof(T)));
    }
    void deallocate(T* p, std::size_t n) noexcept
    {
        _area.deallocate(reinterpret_cast<char*>(p), n*sizeof(T));
    }
    
    template< class U, class... Args >
    void construct(U* p, Args&&... args)
    {
        ::new((void *)p) U(std::forward<Args>(args)...);
    }
    template< class U >
    void destroy(U* p)
    {
        p->~U();
    }

    template <class T1, std::size_t N1, std::size_t A1, 
              class U, std::size_t M, std::size_t A2>
    friend
    bool
    operator==(const StackAllocator<T1, N1, A1>& x, const StackAllocator<U, M, A2>& y) noexcept;

    template <class U, std::size_t M, std::size_t A> friend class StackAllocator;
};

template <class T, std::size_t N, std::size_t A1, class U, std::size_t M, std::size_t A2>
inline
bool
operator==(const StackAllocator<T, N, A1>& x, const StackAllocator<U, M, A2>& y) noexcept
{
    return N == M && A1 == A2 && &x._area == &y._area;
}

template <class T, std::size_t N, std::size_t A1, class U, std::size_t M, std::size_t A2>
inline
bool
operator!=(const StackAllocator<T, N, A1>& x, const StackAllocator<U, M, A2>& y) noexcept
{
    return !(x == y);
}

} // namespace ez

#endif  // __EZ_STACK_ALLOCATOR_HPP__
