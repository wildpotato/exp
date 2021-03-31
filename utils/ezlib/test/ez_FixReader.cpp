#include "catch.hpp"
#include "FixReader.hpp"

#define Delimiter "\x01"

TEST_CASE("FixTagValue - ctor", "[ez][FixReader]")
{
    ez::FixTagValue noValue;
    REQUIRE(noValue.HasValue() == false);
    
    const char *intValueCStr = "-123";
    ez::FixTagValue intValue(intValueCStr, intValueCStr + strlen(intValueCStr));
    REQUIRE(intValue.HasValue() == true);
    REQUIRE(intValue.AsInt() == -123);
    REQUIRE(intValue.AsString() == std::string(intValueCStr));

    const char *uintValueCStr = "456";
    ez::FixTagValue uintValue(uintValueCStr, uintValueCStr + strlen(uintValueCStr));
    REQUIRE(uintValue.HasValue() == true);
    REQUIRE(uintValue.AsInt() == 456);
    REQUIRE(uintValue.AsUInt() == 456);
    REQUIRE(uintValue.AsString() == std::string(uintValueCStr));
}

TEST_CASE("FixTagValue - compare value", "[ez][FixReader]")
{
    const char *valueCStr1 = "456";
    ez::FixTagValue value1(valueCStr1, valueCStr1 + strlen(valueCStr1));
    
    // equal
    REQUIRE(value1.Equal("456") == true);
    REQUIRE(value1.Equal("456", 3) == true);
    
    // not equal
    REQUIRE(value1.Equal("456", 0) == false);
    REQUIRE(value1.Equal("456", 1) == false);
    REQUIRE(value1.Equal("456", 2) == false);
    
    REQUIRE(value1.Equal("455") == false);
    REQUIRE(value1.Equal("455", 3) == false);
    
    REQUIRE(value1.Equal("") == false);
    REQUIRE(value1.Equal("1") == false);
    REQUIRE(value1.Equal("12") == false);
    REQUIRE(value1.Equal("1234") == false);
}

TEST_CASE("FixReader - default ctor", "[ez][FixReader]")
{
    ez::FixReader<128> reader;
    REQUIRE(reader.GetTag(0).HasValue() == false);
    REQUIRE(reader.GetTag(1).HasValue() == false);
    REQUIRE(reader.GetTag(2).HasValue() == false);
}

TEST_CASE("FixReader - parse", "[ez][FixReader]")
{
    ez::FixReader<256> reader;
    
    const char *fixMsg1 = "111=abc" Delimiter "222=-246" Delimiter "123=c" Delimiter "543=1357" Delimiter "333=5566.77" Delimiter;
    size_t fixMsg1Size = strlen(fixMsg1);
    
    auto rc = reader.Parse(fixMsg1, fixMsg1 + fixMsg1Size);
    REQUIRE((bool)rc == true);
    
    REQUIRE(reader.GetTag(111).HasValue() == true);
    REQUIRE(reader.GetTag(111).AsString() == std::string("abc"));
    
    REQUIRE(reader.GetTag(222).HasValue() == true);
    REQUIRE(reader.GetTag(222).AsInt() == -246);

    REQUIRE(reader.GetTag(123).HasValue() == true);
    REQUIRE(reader.GetTag(123).AsChar() == 'c');
    
    REQUIRE(reader.GetTag(543).HasValue() == true);
    REQUIRE(reader.GetTag(543).AsUInt() == 1357);
    
    auto priceValue = reader.GetTag(333);
    auto price = priceValue.AsDouble();
    REQUIRE(priceValue.HasValue() == true);
    REQUIRE(price.first == 556677);
    REQUIRE(price.second == -2);
}

TEST_CASE("FixReader - parse broken msg", "[ez][FixReader]")
{
    ez::FixReader<256> reader;

    // msg with tag without equal
    const char *noEqual = "111=abc" Delimiter "222246" Delimiter;
    size_t noEqualSize = strlen(noEqual);

    ez::ReturnCode rc = reader.Parse(noEqual, noEqual + noEqualSize);
    REQUIRE((bool)rc == false);
    REQUIRE(reader.GetTag(111).HasValue() == true);
    REQUIRE(reader.GetTag(222246).HasValue() == false);
    REQUIRE(reader.GetTag(111).AsString() == std::string("abc"));
    
    // it's ok to have tag with no value
    const char *noValue = "111=abc" Delimiter "222=" Delimiter "333=aaa" Delimiter;
    size_t noValueSize = strlen(noValue);
    
    rc = reader.Parse(noValue, noValue + noValueSize);
    REQUIRE((bool)rc == true);
    REQUIRE(reader.GetTag(111).HasValue() == true);
    REQUIRE(reader.GetTag(222).HasValue() == false);
    REQUIRE(reader.GetTag(333).HasValue() == true);
    REQUIRE(reader.GetTag(111).AsString() == std::string("abc"));
    REQUIRE(reader.GetTag(333).AsString() == std::string("aaa"));
    
    // msg with missing delimiter
    const char *noEnd = "111=abc" Delimiter "222=-246";
    size_t noEndSize = strlen(noEnd);
    
    rc = reader.Parse(noEnd, noEnd + noEndSize);
    REQUIRE((bool)rc == false);
    REQUIRE(reader.GetTag(111).HasValue() == true);
    REQUIRE(reader.GetTag(222).HasValue() == false);
    REQUIRE(reader.GetTag(111).AsString() == std::string("abc"));
}

TEST_CASE("GetFixValue", "[ez][FixReader]")
{
    const char *fixMsg1 = "111=abc" Delimiter "222=-246" Delimiter "123=c" Delimiter "543=1357" Delimiter "333=5566.77" Delimiter;
    
    REQUIRE(ez::GetFixValue(fixMsg1, 111, true).AsString() == "abc");
    REQUIRE(ez::GetFixValue(fixMsg1, 222).AsString() == "-246");
    REQUIRE(ez::GetFixValue(fixMsg1, 123).AsString() == "c");
    REQUIRE(ez::GetFixValue(fixMsg1, 543).AsString() == "1357");
    REQUIRE(ez::GetFixValue(fixMsg1, 333).AsString() == "5566.77");
    
    // tag not exists
    REQUIRE(ez::GetFixValue(fixMsg1, 999).HasValue() == false);
}