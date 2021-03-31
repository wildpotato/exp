#ifndef __EZ_FIX_READER_HPP__
#define __EZ_FIX_READER_HPP__

#include <cassert>
#include <map>
#include <cstring>
#include <string>
#include "StackAllocator.hpp"
#include "ReturnCode.hpp"
#include "FixUtils.hpp"

namespace ez {

// 用來存fix tag的資料位置與資料長度
class FixTagValue
{
public:
    FixTagValue(const char *begin = nullptr, const char *end = nullptr): _begin(begin), _end(end) {}

    bool HasValue() const { return (_begin != nullptr) && (_end != nullptr) && (_begin < _end); }

    const char *Begin() const { return _begin; }
    const char *End() const { return _end; }
    std::size_t Len() const { return (_end - _begin); }
   
    char AsChar() const { return *Begin(); }
    
    template <typename Int_t = std::int32_t>
    Int_t AsInt() const { return ez::atoi<Int_t>(_begin, _end); }
    
    template <typename UInt_t = std::uint32_t>
    UInt_t AsUInt() const { return ez::atou<UInt_t>(_begin, _end); }
    
    template <typename Int_t = std::int32_t>
    std::pair<Int_t, Int_t> AsDouble() const
    {
        std::pair<Int_t, Int_t> result;
        ez::atod(_begin, _end, result.first, result.second);
        return result;
    }
    
    std::string AsString() const { return std::string(_begin, Len()); }
    
    bool Equal(const char *cstr) const
    {
        std::size_t len = std::strlen(cstr);
        return Equal(cstr, len);
    }
    
    bool Equal(const char *data, std::size_t len) const
    {
        if (Len() != len)
            return false;
        else
            return (memcmp(_begin, data, len) == 0);
    }

private:
    const char *_begin; // value在fix訊息中的記憶體起始位置
    const char *_end; // value在fix訊息中的記憶體結束位置
};

// 用來讀取fix message，不支援grouping，只做一次從頭到尾的讀取，讀取過程順便把每個key對應value的位置/大小存起來，方便之後存取
template <std::size_t StackSize>
class FixReader
{
public:
    using FixTag_t = std::uint32_t;
    
    FixReader(): _kvMap(_area) {}
    ~FixReader() {}
    
    // should not be copy/move/assign
    FixReader(const FixReader&) = delete;
    FixReader& operator=(const FixReader&) = delete;
    FixReader(FixReader&&) = delete;
    FixReader& operator=(FixReader&&) = delete;
    
    ReturnCode Parse(const char *begin, const char *end);
    
    const FixTagValue &GetTag(FixTag_t key) const
    {
        static FixTagValue noValue{};
        
        typename _FixKVMap_t::const_iterator i = _kvMap.find(key);
        return (i != _kvMap.end()) ? i->second : noValue;
    }
    
private:
    
    using _FixKVMap_t = std::map<FixTag_t, FixTagValue, std::less<FixTag_t>,
            ez::StackAllocator<std::pair<const FixTag_t, FixTagValue>, StackSize>>;
    
    typename _FixKVMap_t::allocator_type::area_type _area;
    _FixKVMap_t _kvMap;
};  

template <std::size_t StackSize>
ReturnCode FixReader<StackSize>::Parse(const char *begin, const char *end)
{
    _kvMap.clear();
    
    const char *keyBegin(begin);
    while (keyBegin < end)
    {
        const char *keyEnd = (const char *)memchr(keyBegin, '=', end - keyBegin);
        if (keyEnd == NULL)
            return {-1, 0};
        
        const char *valueBegin = keyEnd + 1;
        if (valueBegin >= end)
            return {-2, 0};
        
        const char *valueEnd = (const char *)memchr(valueBegin, '\x01', end - valueBegin);
        if (valueEnd == NULL)
            return {-3, 0};
        
        // store tag in internal stack map
        FixTag_t key = ez::atou<FixTag_t>(keyBegin, keyEnd);
        _kvMap.emplace(key, FixTagValue(valueBegin, valueEnd));
        
        keyBegin = valueEnd + 1;
    }
    
    return {};
}

// get value from fix msg by tag
// if it is 1st msg, it will not have \x01 in front of it
FixTagValue GetFixValue(const char *msg, int tag, bool firstTag = false);

} // namespace ez

#endif // __EZ_FIX_READER_HPP__