#ifndef __EZ_FIX_WRITER_HPP__
#define __EZ_FIX_WRITER_HPP__

#include <cassert>
#include <cstring>
#include <string>
#include "FixUtils.hpp"

namespace ez {

class FixWriter
{
public:
    FixWriter(char *buffer, std::size_t size):
        _begin(buffer), _end(buffer + size), _next(buffer), _bodyLength(nullptr) { }
        
    FixWriter(char *begin, char *end):
        _begin(begin), _end(end), _next(begin), _bodyLength(nullptr) { }
    
    template<std::size_t N>
    FixWriter(char (&buffer)[N]):
        _begin(buffer), _end(&(buffer[N])), _next(buffer), _bodyLength(nullptr) { }
    
    ~FixWriter() {}
    
    // 訊息開頭
    char *MsgBegin() const { return _begin; }
    
    // 訊息塞完后，取得訊息大小
    size_t MsgSize() const { return _next - _begin; }
    
    // 總共可寫入的訊息大小
    size_t BufferSize() const { return _end - _begin; }

    // 剩餘可寫入的訊息大小
    size_t BufferSizeRemaining() const { return _end - _next; }
    
    // 寫入header
    void AddHeader(const char *fixVersion) 
    {
        // 如果被呼叫超過一次的話，_bodyLength不會是nullptr
        assert(_bodyLength == nullptr);
        
        AddStringTag("8", fixVersion);
        
        // 記得body size的位置，等AddTrailer再來寫入size
        _bodyLength = PreFillTag("9", '0', 5);
    }
    
    void AddTrailer()
    {
        assert(_bodyLength != nullptr);
        assert(_bodyLength + 6 <= _next);

        const size_t len = _next - (_bodyLength + 6);
        _bodyLength[0] = '0' + (len / 10000) % 10;
        _bodyLength[1] = '0' + (len / 1000) % 10;
        _bodyLength[2] = '0' + (len / 100) % 10;
        _bodyLength[3] = '0' + (len / 10) % 10;
        _bodyLength[4] = '0' + len % 10;

        const std::uint8_t checksum = ez::GetFIXChecksum(MsgBegin(), MsgSize());

        memcpy(_next, "10=", 3);
        _next += 3;

        _next[0] = '0' + ((checksum / 100) % 10);
        _next[1] = '0' + ((checksum / 10) % 10);
        _next[2] = '0' + (checksum % 10);

        _next += 3;

        AppendDelimiter();
    }
    
    // prefill tag value with value, return value's begin position to write value later
    template<std::size_t N>
    char *PreFillTag(const char(&cstrArray)[N], char value, size_t fillSize)
    {
        AppendString(cstrArray);
        AppendEquality();
        
        // store position and fill value
        char *valuePos = _next;
        memset(_next, value, fillSize);
        _next += fillSize;
        
        AppendDelimiter();
        
        return valuePos;
    }
    char *PreFillTag(std::uint32_t tag, char value, size_t fillSize)
    {
        _next = ez::utoa(tag, _next);
        AppendEquality();

        // store position and fill value
        char *valuePos = _next;
        memset(_next, value, fillSize);
        _next += fillSize;
        
        AppendDelimiter();
        
        return valuePos;
    }
    
    void AppendChar(char ch) { *_next++ = ch; }
    void AppendEquality() { AppendChar('='); }
    void AppendDelimiter() { AppendChar('\x01'); }
    
    void AppendString(const char *cstr)
    {
        std::size_t size = strlen(cstr);
        AppendString(cstr, size);
    }
    void AppendString(const char *data, std::size_t size)
    {
        memcpy(_next, data, size);
        _next += size;
    }
    void AppendString(const std::string &str)
    {
        AppendString(str.c_str(), str.size());
    }
    template<std::size_t N>
    void AppendString(char (&buffer)[N])
    {
        // cstring array include null-terminater, size need -1
        AppendString(buffer, N-1);
    }
    
    void AddStringTag(std::uint32_t tag, const char *buffer, std::size_t size)
    {
        _next = ez::utoa(tag, _next);
        
        AppendEquality();
        
        memcpy(_next, buffer, size);
        _next += size;
        
        AppendDelimiter();
    }
    void AddStringTag(std::uint32_t tag, const char *cstr)
    {
        size_t size = strlen(cstr);
        AddStringTag(tag, cstr, size);
    }
    void AddStringTag(std::uint32_t tag, const std::string &str)
    {
        AddStringTag(tag, str.c_str(), str.size());
    }
    template<size_t N>
    void AddStringTag(const char(&cstrArray)[N], const char *buffer, std::size_t size) 
    {
        AppendString(cstrArray);
        AppendEquality();
        AppendString(buffer, size);
        AppendDelimiter();
    }
    template<size_t N>
    void AddStringTag(const char(&cstrArray)[N], const char *cstr) 
    {
        AddStringTag(cstrArray, cstr, strlen(cstr));
    }
    template<size_t N>
    void AddStringTag(const char(&cstrArray)[N], const std::string &str) 
    {
        AddStringTag(cstrArray, str.c_str(), str.size());
    }
    
    void AddCharTag(std::uint32_t tag, char character)
    {
        _next = ez::utoa(tag, _next);
        AppendEquality();
        *_next++ = character;
        AppendDelimiter();
    }
    template<size_t N>
    void AddCharTag(const char(&cstrArray)[N], char character)
    {
        AppendString(cstrArray);
        AppendEquality();
        *_next++ = character;
        AppendDelimiter();
    }
    
    template<typename Int_type>
    void AddIntTag(std::uint32_t tag, Int_type number) 
    {
        _next = ez::utoa(tag, _next);
        AppendEquality();
        _next = ez::itoa(number, _next);
        AppendDelimiter();
    }
    template<typename Int_type, size_t N>
    void AddIntTag(const char(&cstrArray)[N], Int_type number)
    {
        AppendString(cstrArray);
        AppendEquality();
        _next = ez::itoa(number, _next);
        AppendDelimiter();
    }
    
    template<typename UInt_type>
    void AddUIntTag(std::uint32_t tag, UInt_type number) 
    {
        assert(number >= 0);
        _next = ez::utoa(tag, _next);
        AppendEquality();
        _next = ez::utoa(number, _next);
        AppendDelimiter();
    }
    template<typename UInt_type, size_t N>
    void AddUIntTag(const char(&cstrArray)[N], UInt_type number)
    {
        assert(number >= 0);
        AppendString(cstrArray);
        AppendEquality();
        _next = ez::utoa(number, _next);
        AppendDelimiter();
    }
    
    template<typename UInt_type>
    void AddUIntTag(std::uint32_t tag, UInt_type number, std::size_t padSize) 
    {
        assert(number >= 0);
        _next = ez::utoa(tag, _next);
        AppendEquality();
        _next = ez::utoa_fill0(number, _next, padSize);
        AppendDelimiter();
    }
    template<typename UInt_type, size_t N>
    void AddUIntTag(const char(&cstrArray)[N], UInt_type number, std::size_t padSize)
    {
        assert(number >= 0);
        AppendString(cstrArray);
        AppendEquality();
        _next = ez::utoa_fill0(number, _next, padSize);
        AppendDelimiter();
    }
    
    // 用來寫otc的價格欄位 9(5).9(4), 因為otc是固定長度的，所以要前面補0
    template<typename UInt_type>
    void AddPrice100Tag(std::uint32_t tag, UInt_type price100) 
    {
        assert(price100 >= 0);
        _next = ez::utoa(tag, _next);
        AppendEquality();
        _next = ez::utoa_fill0(price100 / 100, _next, 5);
        AppendChar('.');
        _next = ez::utoa_fill0(price100 % 100, _next, 2);
        AppendString("00", 2);
        AppendDelimiter();
    }
    template<typename UInt_type, size_t N>
    void AddPrice100Tag(const char(&cstrArray)[N], UInt_type price100) 
    {
        assert(price100 >= 0);
        AppendString(cstrArray);
        AppendEquality();
        _next = ez::utoa_fill0(price100 / 100, _next, 5);
        AppendChar('.');
        _next = ez::utoa_fill0(price100 % 100, _next, 2);
        AppendString("00", 2);
        AppendDelimiter();
    }
    
    // exponent <= 0
    template<typename Int_type>
    void AddDecimal(std::uint32_t tag, Int_type mantissa, Int_type exponent) 
    {
        _next = ez::utoa(tag, _next);
        AppendEquality();
        _next = ez::dtoa(mantissa, exponent, _next);
        AppendDelimiter();
    }
    template<typename Int_type, std::size_t N>
    void AddDecimal(const char(&cstrArray)[N], Int_type mantissa, Int_type exponent) 
    {
        AppendString(cstrArray);
        AppendEquality();
        _next = ez::dtoa(mantissa, exponent, _next);
        AppendDelimiter();
    }
    
    
private:
    char* _begin;
    char* _end;
    // 目前用來寫入新資料位置
    char* _next;
    // 指向用來寫BodyLength的地方，目前是用6位數，最大999999
    char* _bodyLength;
};

} // namespace ez

#endif // __EZ_FIX_WRITER_HPP__