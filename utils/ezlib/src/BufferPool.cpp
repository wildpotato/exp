#include "BufferPool.hpp"
#include "boost/pool/singleton_pool.hpp"

namespace ez {

struct _Buffer128Tag { };
struct _Buffer256Tag { };
struct _Buffer512Tag { };
struct _Buffer1KTag { };
struct _Buffer2KTag { };
struct _Buffer4KTag { };

typedef boost::singleton_pool<_Buffer128Tag, 128> GBuffer128Pool;
typedef boost::singleton_pool<_Buffer256Tag, 256> GBuffer256Pool;
typedef boost::singleton_pool<_Buffer512Tag, 512> GBuffer512Pool;
typedef boost::singleton_pool<_Buffer1KTag, 1024> GBuffer1KPool;
typedef boost::singleton_pool<_Buffer2KTag, 2048> GBuffer2KPool;
typedef boost::singleton_pool<_Buffer4KTag, 4096> GBuffer4KPool;

Buffer Get128Buffer()
{
    char *buffer = (char *)GBuffer128Pool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer128Pool::free(p); });
    return { std::move(bufferPtr), 128 };
}

Buffer Get256Buffer()
{
    char *buffer = (char *)GBuffer256Pool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer256Pool::free(p); });
    return { std::move(bufferPtr), 256 };
}

Buffer Get512Buffer()
{
    char *buffer = (char *)GBuffer512Pool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer512Pool::free(p); });
    return { std::move(bufferPtr), 512 };
}

Buffer Get1KBuffer()
{
    char *buffer = (char *)GBuffer1KPool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer1KPool::free(p); });
    return { std::move(bufferPtr), 1024 };
}

Buffer Get2KBuffer()
{
    char *buffer = (char *)GBuffer2KPool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer2KPool::free(p); });
    return { std::move(bufferPtr), 2048 };
}

Buffer Get4KBuffer()
{
    char *buffer = (char *)GBuffer4KPool::malloc();
    RawDataPtr bufferPtr(buffer, [] (char *p) { GBuffer4KPool::free(p); });
    return { std::move(bufferPtr), 4096 };
}

Buffer GetBuffer(unsigned int size)
{
    char *buffer = new char[size];
    RawDataPtr bufferPtr(buffer, std::default_delete<char[]>());
    return { std::move(bufferPtr), size };
}

Buffer GetBuffer_Align(unsigned int size)
{
    if (size <= 128)
        return Get128Buffer();
    else if (size <= 256)
        return Get256Buffer();
    else if (size <= 512)
        return Get512Buffer();
    else if (size <= 1024)
        return Get1KBuffer();
    else if (size <= 2048)
        return Get2KBuffer();
    else if (size <= 4096)
        return Get4KBuffer();
    else
        return GetBuffer(size);
}

} // namespace ez
