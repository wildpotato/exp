#include "catch.hpp"
#include "FixUtils.hpp"
#include <cstdlib>
#include <cstring>

TEST_CASE("FixUtils - atoi", "[ez][FixUtils]")
{
    const char *num1 = "12345";
    const char *num2 = "123456789";
    const char *negnum1 = "-12345";
    const char *negnum2 = "-123456789";

    // buffer data case
    REQUIRE(ez::atoi<int>(num1, num1+5) == atoi(num1));
    REQUIRE(ez::atoi<int>(num2, num2+9) == atoi(num2));
    REQUIRE(ez::atoi<int>(negnum1, negnum1+6) == atoi(negnum1));
    REQUIRE(ez::atoi<int>(negnum2, negnum2+10) == atoi(negnum2));
    
    // c-str case
    REQUIRE(ez::atoi<int>(num1) == atoi(num1));
    REQUIRE(ez::atoi<int>(num2) == atoi(num2));
    REQUIRE(ez::atoi<int>(negnum1) == atoi(negnum1));
    REQUIRE(ez::atoi<int>(negnum2) == atoi(negnum2));
    
    // null string
    REQUIRE(ez::atoi<int>(num1, num1) == 0);
    REQUIRE(ez::atoi<int>("") == 0);
}

TEST_CASE("FixUtils - atou", "[ez][FixUtils]")
{
    const char *num1 = "12345";
    const char *num2 = "123456789";

    // buffer data case
    REQUIRE(ez::atou<unsigned int>(num1, num1+5) == atoi(num1));
    REQUIRE(ez::atou<unsigned int>(num2, num2+9) == atoi(num2));
    
    // c-str case
    REQUIRE(ez::atou<unsigned int>(num1) == atoi(num1));
    REQUIRE(ez::atou<unsigned int>(num2) == atoi(num2));
    
    // null string
    REQUIRE(ez::atou<unsigned int>(num1, num1) == 0);
    REQUIRE(ez::atou<unsigned int>("") == 0);
}

TEST_CASE("FixUtils - itoa", "[ez][FixUtils]")
{
    char buffer[16];
    char *end(nullptr);
    
    // positive
    end = ez::itoa(12345, buffer);
    REQUIRE(memcmp(buffer, "12345", 5) == 0);
    REQUIRE(end == buffer + 5);
    
    // negative
    end = ez::itoa(-12345, buffer);
    REQUIRE(memcmp(buffer, "-12345", 6) == 0);
    REQUIRE(end == buffer + 6);
    
    // zero
    end = ez::itoa(0, buffer);
    REQUIRE(memcmp(buffer, "0", 1) == 0);
    REQUIRE(end == buffer + 1);
}

TEST_CASE("FixUtils - utoa", "[ez][FixUtils]")
{
    char buffer[16];
    char *end(nullptr);
    
    // no padding
    end = ez::utoa(12345, buffer);
    REQUIRE(memcmp(buffer, "12345", 5) == 0);
    REQUIRE(end == buffer + 5);
    
    // left 0 padding
    end = ez::utoa_fill0(12345, buffer, 10);
    REQUIRE(memcmp(buffer, "0000012345", 10) == 0);
    REQUIRE(end == buffer + 10);
    end = ez::utoa_fill0(12345, buffer, 3);
    REQUIRE(memcmp(buffer, "345", 3) == 0);
    REQUIRE(end == buffer + 3);
    
    // zero
    end = ez::utoa_fill0(0, buffer, 3);
    REQUIRE(memcmp(buffer, "000", 3) == 0);
    REQUIRE(end == buffer + 3);
}

TEST_CASE("FixUtils - atod", "[ez][FixUtils]")
{
    int mantissa(0);
    int exponent(0);
    
    // positive
    const char *cstr = "123.450";
    ez::atod(cstr, cstr + strlen(cstr), mantissa, exponent);
    REQUIRE(mantissa == 123450);
    REQUIRE(exponent == -3);

    // negtive
    cstr = "-123.45";
    ez::atod(cstr, cstr + strlen(cstr), mantissa, exponent);
    REQUIRE(mantissa == -12345);
    REQUIRE(exponent == -2);
    
    // integer
    cstr = "12345";
    ez::atod(cstr, cstr + strlen(cstr), mantissa, exponent);
    REQUIRE(mantissa == 12345);
    REQUIRE(exponent == 0);
    
    // zero
    cstr = "0";
    ez::atod(cstr, cstr + strlen(cstr), mantissa, exponent);
    REQUIRE(mantissa == 0);
    REQUIRE(exponent == 0);
    cstr = "000";
    ez::atod(cstr, cstr + strlen(cstr), mantissa, exponent);
    REQUIRE(mantissa == 0);
    REQUIRE(exponent == 0);
    
    // null-string
    ez::atod(cstr, cstr, mantissa, exponent);
    REQUIRE(mantissa == 0);
    REQUIRE(exponent == 0);
}

TEST_CASE("FixUtils - dtoa", "[ez][FixUtils]")
{
    char tmp[16];
    char *end(nullptr);
    
    // positive
    int num(123456);
    int exp(-2);
    end = ez::dtoa(num, exp, tmp);
    REQUIRE(memcmp(tmp, "1234.56", 7) == 0);
    REQUIRE(end == tmp + 7);
    
    // negtive
    num = -12345;
    exp = -3;
    end = ez::dtoa(num, exp, tmp);
    REQUIRE(memcmp(tmp, "-12.345", 7) == 0);
    REQUIRE(end == tmp + 7);
    
    // integer
    num = 12345;
    exp = 0;
    end = ez::dtoa(num, exp, tmp);
    REQUIRE(memcmp(tmp, "12345", 5) == 0);
    REQUIRE(end == tmp + 5);
    
    // zero
    num = 0;
    exp = 0;
    end = ez::dtoa(num, exp, tmp);
    REQUIRE(memcmp(tmp, "0", 1) == 0);
    REQUIRE(end == tmp + 1);
}

TEST_CASE("FixUtils - GetOtcPrice100", "[ez][FixUtils]")
{
    REQUIRE(ez::GetOtcPrice100<int>("00000.0000") == 0);
    REQUIRE(ez::GetOtcPrice100<int>("00050.0012") == 5000);
    REQUIRE(ez::GetOtcPrice100<int>("00246.0123") == 24601);
    REQUIRE(ez::GetOtcPrice100<int>("12345.6789") == 1234567);
    REQUIRE(ez::GetOtcPrice100<int>("99999.9999") == 9999999);
}

TEST_CASE("FixUtils - checksum", "[ez][FixUtils]")
{
    const char *test1 = "i1u2h09fasdfb";
    REQUIRE(ez::GetFIXChecksum(test1, strlen(test1)) == ::atoi(ez::GetDefaultFixCheckSum(test1, strlen(test1))));
    
    const char *test2 = ";ovasuasds9vh2";
    REQUIRE(ez::GetFIXChecksum(test2, strlen(test2)) == ::atoi(ez::GetDefaultFixCheckSum(test2, strlen(test2))));
    
    const char *test3 = "d9o8fvbn3grg4go2u4hgf89";
    REQUIRE(ez::GetFIXChecksum(test3, strlen(test3)) == ::atoi(ez::GetDefaultFixCheckSum(test3, strlen(test3))));
}