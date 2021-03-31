#ifndef __EZ_FIXED_STRING_HPP__
#define __EZ_FIXED_STRING_HPP__

#include <string>
#include <cstring>
#include <cassert>
#include "boost/functional/hash.hpp"

namespace ez {

// c-string with fixed buffer size
template <int BUFFER_SIZE>
class FixedString
{
public:
    FixedString() { Clear(); }

    FixedString(const char *cstr) { SetStr(cstr, strlen(cstr)); }

    FixedString(const char *cstr, size_t len) { SetStr(cstr, len); }

    FixedString(const std::string &str) { SetStr(str.c_str(), str.size()); }

    ~FixedString() {}

    void SetStr(const char *cstr)
    {
        _SetString(cstr, strlen(cstr));
    }

    void SetStr(const char *cstr, size_t len)
    {
        _SetString(cstr, len);
    }

    void SetStr(const std::string &str)
    {
        _SetString(str.c_str(), str.size());
    }
    
    void Clear() { SetStr("", 0); }

    const char *CStr() const { return _data; }

    std::string Str() const { return std::string(_data, _len); }

    size_t Size() const { return _len; }

    size_t MaxSize() const { return BUFFER_SIZE - 1; }

    size_t BufferSize() const { return BUFFER_SIZE; }

    bool operator==(const FixedString &target) const
    {
        return strcmp(_data, target._data) == 0;
    }

    bool operator!=(const FixedString &target) const
    {
        return strcmp(_data, target._data) != 0;
    }

    bool operator<(const FixedString &target) const
    {
        return strcmp(_data, target._data) < 0;
    }

    bool operator>(const FixedString &target) const
    {
        return strcmp(_data, target._data) > 0;
    }

    bool operator<=(const FixedString &target) const
    {
        return strcmp(_data, target._data) <= 0;
    }

    bool operator>=(const FixedString &target) const
    {
        return strcmp(_data, target._data) >= 0;
    }
    
    void TrimRight(char trimChar = ' ')
    {
        size_t count = _len;
        while (count > 0)
        {
            char &lastChar = _data[--count];
            if (lastChar == trimChar)
            {
                lastChar = '\0';
                --_len;
            }
            else
                break;
        }
    }

    std::size_t GetHash() const
    {
        return boost::hash_range(_data, _data + _len);
    }

    char *_RawData() { return _data; }

    void _SetSize(size_t size) { _len = size; }

private:

    void _SetString(const char *cstr, size_t len)
    {
        _len = (len < BUFFER_SIZE) ? len: MaxSize();

        memcpy(_data, cstr, _len);
        _data[_len] = '\0';
    }

    char _data[BUFFER_SIZE];
    size_t _len;
};

typedef FixedString<8> String8;
typedef FixedString<12> String12;
typedef FixedString<16> String16;
typedef FixedString<24> String24;
typedef FixedString<32> String32;
typedef FixedString<64> String64;
typedef FixedString<128> String128;

} // namespace ez

// hash function for unordered container
namespace std {

    template <int BUFFER_SIZE>
    struct hash<ez::FixedString<BUFFER_SIZE>>
    {
        std::size_t operator()(const ez::FixedString<BUFFER_SIZE>& k) const
        {
            return k.GetHash();
        }
    };
}

#endif // __EZ_FIXED_STRING_HPP__