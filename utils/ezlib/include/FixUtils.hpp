#ifndef __EZ_FIX_UTILS_HPP__
#define __EZ_FIX_UTILS_HPP__

#include <cassert>
#include <algorithm>

namespace ez {

// get int from buffer data, given buffer's begin/end
template<typename Int_type> Int_type atoi(const char *begin, const char *end)
{
    assert(begin <= end);

    bool isnegative(false);
    if (*begin == '-') 
    {
        isnegative = true;
        ++begin;
    }

    Int_type val(0);
    for(; begin < end; ++begin) 
    {
        val *= 10;
        val += (Int_type)(*begin - '0');
    }

    return isnegative ? -val : val;
}
template<typename Int_type> Int_type atoi(const char *cstr)
{
    bool isnegative(false);
    if (*cstr == '-') {
        isnegative = true;
        ++cstr;
    }

    Int_type val(0);
    for (; *cstr != '\0'; ++cstr)
    {
        val *= 10;
        val += (Int_type)(*cstr - '0');        
    }
    
    return isnegative ? -val : val;
}

// get unsigned int from buffer data
template<typename Uint_type> Uint_type atou(char const* begin, char const* end)
{
    assert(begin <= end);
    
    Uint_type val(0);
    for(; begin < end; ++begin)
    {
        val *= 10u;
        val += (Uint_type)(*begin - '0');
    }

    return val;
}
template<typename Uint_type> Uint_type atou(const char *cstr)
{
    Uint_type val(0);
    for (; *cstr != '\0'; ++cstr)
    {
        val *= 10u;
        val += (Uint_type)(*cstr - '0');        
    }
    
    return val;
}

// convert int to c-string, no null at end, return position after last char
template<typename Int_type> char *itoa(Int_type number, char *output)
{
    bool isnegative(false);
    if (number < 0) {
        isnegative = true;
        number = -number;
    }

    char *b = output;
    do {
        *b++ = '0' + (number % 10);
        number /= 10;
    } while(number);

    if (isnegative) {
        *b++ = '-';
    }

    // Reverse the digits in-place.
    std::reverse(output, b);
    return b;
}

// convert unsigned int to c-string, no null at end, return position after last char
template<typename Uint_type> char *utoa(Uint_type number, char *output)
{
    char *b = output;
    do {
        *b++ = '0' + (number % 10);
        number /= 10;
    } while(number);

    // Reverse the digits in-place.
    std::reverse(output, b);
    return b;
}
// convert unsigned int to c-string, padding '0' at left, no null at end
template<typename Uint_type> char *utoa_fill0(Uint_type number, char *output, std::size_t padSize)
{
    for (std::size_t i = 0; i < padSize; ++i) 
    {
        output[i] = '0' + (number % 10);
        number /= 10;
    }

    char *end = output + padSize;
    std::reverse(output, output + padSize);
    return end;
}

// convert data buffer to mantissa+exponent
template<typename Int_type> void atod(const char *begin, const char *end, Int_type& mantissa, Int_type& exponent)
{
    assert(begin <= end);
    
    Int_type mantissa_ = 0;
    Int_type exponent_ = 0;
    bool isdecimal(false);
    bool isnegative(false);

    if (*begin == '-') {
        isnegative = true;
        ++begin;
    }

    for(; begin < end; ++begin) 
    {
        if (*begin == '.') 
        {
            isdecimal = true;
        }
        else 
        {
            mantissa_ *= 10;
            mantissa_ += (*begin - '0');
            
            if (isdecimal)
                --exponent_;
        }
    }

    if (isnegative)
        mantissa_ = -mantissa_;
    
    mantissa = mantissa_;
    exponent = exponent_;
}

// convert decimal to str, exponent <= 0
template<typename Int_type> char* dtoa(Int_type mantissa, Int_type exponent, char *buffer)
{
    bool isnegative(false);
    if (mantissa < 0) {
        isnegative = true;
        mantissa = -mantissa;
    }

    char *b = buffer;
    do {
        *b++ = '0' + (mantissa % 10);
        mantissa /= 10;
        
        if (++exponent == 0)
            *b++ = '.';

    } while (mantissa > 0 || exponent < 1);

    if (isnegative)
        *b++ = '-';

    // Reverse the digits in-place.
    std::reverse(buffer, b);
    return b;
}

// 專門解出OTC的價格欄位，直接回傳 price100 的數值，你必須確認字串長度足夠
template<typename Int_type> Int_type GetOtcPrice100(const char *otcPriceStr)
{
    typedef typename std::make_unsigned<Int_type>::type Uint_type;
    
    // 目前價格欄位格式為 9(5).9(4)，捨棄最後兩位
    Uint_type integerPart = atou<Uint_type>(otcPriceStr, otcPriceStr + 5);
    Uint_type fractionalPart = atou<Uint_type>(otcPriceStr + 6, otcPriceStr + 8);
    return (integerPart * 100) + fractionalPart;
}

// fix規格上面的code, 用來在test case驗證自己的checksum
inline char *GetDefaultFixCheckSum(const char *buf, long bufLen)
{
    static char tmpBuf[4];
    long idx;
    unsigned int cks;

    for( idx = 0L, cks = 0; idx < bufLen; cks += (unsigned int)buf[ idx++ ] );
    sprintf( tmpBuf, "%03d", (unsigned int)( cks % 256 ) );
    return tmpBuf;	
}

inline std::uint8_t GetFIXChecksum(const char *buf, int size)
{
    int remainder = size % 4;

    std::uint8_t subSum1(0);
    std::uint8_t subSum2(0);
    std::uint8_t subSum3(0);
    std::uint8_t subSum4(0);

    const std::uint8_t *data = (const std::uint8_t *)buf;
    switch (remainder) {
    case 1:
        subSum1 += data[0];
        break;
    case 2:
        subSum1 += data[0];
        subSum2 += data[1];
        break;
    case 3:
        subSum1 += data[0];
        subSum2 += data[1];
        subSum3 += data[2];
        break;
    case 0:
        break;
    }

    for (int i = remainder; i < size; i += 4) {
        subSum1 += data[i];
        subSum2 += data[i + 1];
        subSum3 += data[i + 2];
        subSum4 += data[i + 3];
    }

    return (subSum1 + subSum2 + subSum3 + subSum4);
}

} // namespace ez

#endif // __EZ_FIX_UTILS_HPP__