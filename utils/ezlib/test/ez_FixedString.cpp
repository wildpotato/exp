#include "catch.hpp"
#include <unordered_map>
#include "FixedString.hpp"

using String8 = ez::String8;
using String16 = ez::String16;

TEST_CASE("FixedString constructor", "[ez][FixedString]")
{
    SECTION("default constructor") 
    {
        String8 str;

        REQUIRE(strcmp(str.CStr(), "") == 0);
        REQUIRE(str.Str() == std::string());
        REQUIRE(str.Size() == 0);
        REQUIRE(str.MaxSize() == 7);
        REQUIRE(str.BufferSize() == 8);   
    }

    SECTION("constructor by cstring") 
    {
        const char *cstr = "12345";
        String8 str(cstr);

        REQUIRE(strcmp(str.CStr(), cstr) == 0);
        REQUIRE(str.Str() == std::string(cstr));
        REQUIRE(str.Size() == strlen(cstr));
        REQUIRE(str.MaxSize() == 7);
        REQUIRE(str.BufferSize() == 8);   
    }

    SECTION("constructor by over-sized cstring") 
    {
        const char *cstr = "1234567890";
        const char *expectCStr = "1234567";
        String8 str(cstr);

        REQUIRE(strcmp(str.CStr(), expectCStr) == 0);
        REQUIRE(str.Str() == std::string(expectCStr));
        REQUIRE(str.Size() == strlen(expectCStr));
        REQUIRE(str.MaxSize() == 7);
        REQUIRE(str.BufferSize() == 8);   
    }
}

TEST_CASE("FixedString assignment", "[ez][FixedString]")
{
        const char *cstr = "123456";
        String8 str(cstr);
        String8 targetStr;
        
        targetStr = str;
        REQUIRE(strcmp(targetStr.CStr(), str.CStr()) == 0);
        REQUIRE(targetStr.Size() == str.Size());
        REQUIRE(targetStr.GetHash() == str.GetHash());
}

TEST_CASE("FixedString trim", "[ez][FixedString]")
{
    SECTION("trim empty")
    {
        String8 str("");
        str.TrimRight();
        REQUIRE(str.Size() == 0);
        REQUIRE(str.Str() == std::string());
    }
    
    SECTION("no char to trim")
    {
        String8 str("123");
        str.TrimRight();
        REQUIRE(str.Size() == 3);
        REQUIRE(str.Str() == "123");
    }
    
    SECTION("trim ok")
    {
        String8 str("123 ");
        str.TrimRight();
        REQUIRE(str.Size() == 3);
        REQUIRE(str.Str() == "123");
    }
    
    SECTION("trim multi char ok")
    {
        String8 str("123   ");
        str.TrimRight();
        REQUIRE(str.Size() == 3);
        REQUIRE(str.Str() == "123");
    }

    SECTION("trim certain char ok")
    {
        String8 str("123--");
        str.TrimRight('-');
        REQUIRE(str.Size() == 3);
        REQUIRE(str.Str() == "123");
    }
    
    SECTION("trim stop correctly")
    {
        String16 str("123 456 7  ");
        str.TrimRight();
        REQUIRE(str.Size() == 9);
        REQUIRE(str.Str() == "123 456 7");
    }
}

TEST_CASE("FixedString hash code", "[ez][FixedString]")
{
    String8 str1("12345");
    String8 str2("12345");
    String8 str3("1234567");

    SECTION("same string value should have same hash value")
    {
        REQUIRE(str1.GetHash() == str2.GetHash());
    }
    
    SECTION("diff string value should have diff hash value")
    {
        REQUIRE(str1.GetHash() != str3.GetHash());
        REQUIRE(str2.GetHash() != str3.GetHash());
    }

    SECTION("diff string size type, but have same value should still have same hash value")
    {
        String16 largerStr(str3.CStr(), str3.Size());
        REQUIRE(str3.GetHash() == largerStr.GetHash());
    }

    SECTION("trimmed string should still have same hash value")
    {
        String8 trimmedStr("1234567890");
        REQUIRE(str3.GetHash() == trimmedStr.GetHash());
    }

    SECTION("usable on std::unordered_map")
    {
        std::unordered_map<String8, int> map;

        auto i = map.find(str1);
        REQUIRE(i == map.end());

        // insert key/value, and find it
        map[str1] = 1;
        i = map.find(str1);
        REQUIRE(i != map.end());
        REQUIRE(i->first == str1);
        REQUIRE(i->second == 1);

        // find by same key
        i = map.find(str1);
        REQUIRE(i != map.end());
        REQUIRE(i->first == str1);
        REQUIRE(i->second == 1);

        // find by same value
        i = map.find(str2);
        REQUIRE(i != map.end());
        REQUIRE(i->first == str1);
        REQUIRE(i->first == str2);
        REQUIRE(i->second == 1);

        // find by c-str
        i = map.find("12345");
        REQUIRE(i != map.end());
        REQUIRE(i->first == str1);
        REQUIRE(i->first == str2);
        REQUIRE(i->second == 1);
    }

}