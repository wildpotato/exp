#ifndef __EZ_BUFFER_POOL_HPP__
#define __EZ_BUFFER_POOL_HPP__

#include <memory>

namespace ez {

typedef std::shared_ptr<char> RawDataPtr;

class Buffer
{
public:
    Buffer(): _maxSize(0) {}
    Buffer(const RawDataPtr &ptr, std::size_t size)
        : _rawPtr(ptr), _maxSize(size) {}

    Buffer(RawDataPtr &&ptr, std::size_t size)
        : _rawPtr(std::move(ptr)), _maxSize(size) {}
    
    Buffer(const Buffer &buffer) = default;
    Buffer(Buffer &&buffer) = default;
    Buffer &operator=(const Buffer &buffer) = default;
    Buffer &operator=(Buffer &&buffer) = default;
    ~Buffer() = default;
    
    explicit operator bool() const noexcept { return IsValid(); }
    bool IsValid() const { return bool(_rawPtr); }
    
    char *Begin() const { return _rawPtr.get(); }
    char *End() const { return _rawPtr.get() + GetMaxSize(); }
    
    std::size_t GetMaxSize() const { return _maxSize; }
    
private:
    RawDataPtr _rawPtr;
    std::size_t _maxSize;
};

// get buffer with size known at compile-time
Buffer Get128Buffer();
Buffer Get256Buffer();
Buffer Get512Buffer();
Buffer Get1KBuffer();
Buffer Get2KBuffer();
Buffer Get4KBuffer();

// get buffer with size only known at run-time, directly use "new []" to create buffer
Buffer GetBuffer(unsigned int size);
// get buffer with size only known at run-time, this will use proper buffer pool if exists
Buffer GetBuffer_Align(unsigned int size);

} // namespace ez

#endif // __EZ_BUFFER_POOL_HPP__