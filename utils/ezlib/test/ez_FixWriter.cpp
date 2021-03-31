#include "catch.hpp"
#include "FixWriter.hpp"

#define Delimiter "\x01"

TEST_CASE("FixWriter - ctor", "[ez][FixWriter]")
{
    char buffer[16] = {0};
    size_t bufferSize= sizeof(buffer);
    
    ez::FixWriter msg1(buffer, bufferSize);
    REQUIRE(msg1.MsgBegin() == buffer);
    REQUIRE(msg1.MsgSize() == 0);
    REQUIRE(msg1.BufferSize() == bufferSize);
    REQUIRE(msg1.BufferSizeRemaining() == bufferSize);
    
    ez::FixWriter msg2(buffer, buffer + bufferSize);
    REQUIRE(msg2.MsgBegin() == buffer);
    REQUIRE(msg2.MsgSize() == 0);
    REQUIRE(msg2.BufferSize() == bufferSize);
    REQUIRE(msg2.BufferSizeRemaining() == bufferSize);
    
    ez::FixWriter msg3(buffer);
    REQUIRE(msg3.MsgBegin() == buffer);
    REQUIRE(msg3.MsgSize() == 0);
    REQUIRE(msg3.BufferSize() == bufferSize);
    REQUIRE(msg3.BufferSizeRemaining() == bufferSize);
}

TEST_CASE("FixWriter - Header/Trailer", "[ez][FixWriter]")
{
    char buffer[64] = {0};
    const char *target = "8=FIX.4.3" Delimiter "9=00000" Delimiter;
    size_t targetSize = strlen(target);

    ez::FixWriter msg(buffer);
    msg.AddHeader("FIX.4.3");
    REQUIRE(memcmp(msg.MsgBegin(), target, targetSize) == 0);
    REQUIRE(msg.MsgSize() == targetSize);
    REQUIRE(msg.BufferSizeRemaining() == sizeof(buffer) - targetSize);
    
    INFO("checksum: " << ez::GetDefaultFixCheckSum(msg.MsgBegin(), msg.MsgSize()));
    
    target = "8=FIX.4.3" Delimiter "9=00000" Delimiter "10=135" Delimiter;
    targetSize = strlen(target);
    msg.AddTrailer();
    REQUIRE(memcmp(msg.MsgBegin(), target, targetSize) == 0);
    REQUIRE(msg.MsgSize() == targetSize);
    REQUIRE(msg.BufferSizeRemaining() == sizeof(buffer) - targetSize);
}

TEST_CASE("FixWriter - StringTag", "[ez][FixWriter]")
{
    char buffer[64] = {0};
    
    // data + size
    const char *target = "123=abc123" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddStringTag(123, "abc123", 6);
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    // cstring
    target = "222=abcdefg" Delimiter;
    ez::FixWriter msg2(buffer);
    msg2.AddStringTag(222, "abcdefg");
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    // std::string
    target = "3333=value" Delimiter;
    ez::FixWriter msg3(buffer);
    msg3.AddStringTag(3333, std::string("value"));
    REQUIRE(memcmp(msg3.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg3.MsgSize() == strlen(target));
    REQUIRE(msg3.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    // tag as string literal
    target = "44=qwerty" Delimiter;
    ez::FixWriter msg4(buffer);
    msg4.AddStringTag("44", "qwerty");
    REQUIRE(memcmp(msg4.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg4.MsgSize() == strlen(target));
    REQUIRE(msg4.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
}

TEST_CASE("FixWriter - CharTag", "[ez][FixWriter]")
{
    char buffer[32] = {0};
    
    const char *target = "123=a" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddCharTag(123, 'a');
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    // tag as string literal
    ez::FixWriter msg2(buffer);
    msg2.AddCharTag("123", 'a');
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
}

TEST_CASE("FixWriter - IntTag", "[ez][FixWriter]")
{
    char buffer[32] = {0};
    
    // positive
    const char *target = "123=1357" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddIntTag(123, 1357);
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    // negtive
    target = "123=-24680" Delimiter;
    ez::FixWriter msg2(buffer);
    msg2.AddIntTag(123, -24680);
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    // zero
    target = "123=0" Delimiter;
    ez::FixWriter msg3(buffer);
    msg3.AddIntTag(123, 0);
    REQUIRE(memcmp(msg3.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg3.MsgSize() == strlen(target));
    REQUIRE(msg3.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    // tag as string literal
    target = "123=543" Delimiter;
    ez::FixWriter msg4(buffer);
    msg4.AddIntTag("123", 543);
    REQUIRE(memcmp(msg4.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg4.MsgSize() == strlen(target));
    REQUIRE(msg4.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
}

TEST_CASE("FixWriter - UIntTag", "[ez][FixWriter]")
{
    char buffer[32] = {0};
    
    const char *target = "123=1357" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddUIntTag(123, 1357);
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    target = "123=0024680" Delimiter;
    ez::FixWriter msg2(buffer);
    msg2.AddUIntTag(123, 24680, 7);
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    target = "123=0" Delimiter;
    ez::FixWriter msg3(buffer);
    msg3.AddUIntTag(123, 0);
    REQUIRE(memcmp(msg3.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg3.MsgSize() == strlen(target));
    REQUIRE(msg3.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    target = "123=00000" Delimiter;
    ez::FixWriter msg4(buffer);
    msg4.AddUIntTag(123, 0, 5);
    REQUIRE(memcmp(msg4.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg4.MsgSize() == strlen(target));
    REQUIRE(msg4.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    // tag as string literal
    target = "123=246" Delimiter;
    ez::FixWriter msg5(buffer);
    msg5.AddUIntTag("123", 246);
    REQUIRE(memcmp(msg5.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg5.MsgSize() == strlen(target));
    REQUIRE(msg5.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 

    target = "123=00246" Delimiter;
    ez::FixWriter msg6(buffer);
    msg6.AddUIntTag("123", 246, 5);
    REQUIRE(memcmp(msg6.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg6.MsgSize() == strlen(target));
    REQUIRE(msg6.BufferSizeRemaining() == sizeof(buffer) - strlen(target));     
}

TEST_CASE("FixWriter - PriceTag", "[ez][FixWriter]")
{
    char buffer[32] = {0};
    
    const char *target = "123=00234.5600" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddPrice100Tag(123, 23456);
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    target = "123=00246.0800" Delimiter;
    ez::FixWriter msg2(buffer);
    msg2.AddPrice100Tag(123, 24608);
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    // tag as string literal
    ez::FixWriter msg3(buffer);
    msg3.AddPrice100Tag("123", 24608);
    REQUIRE(memcmp(msg3.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg3.MsgSize() == strlen(target));
    REQUIRE(msg3.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
}

TEST_CASE("FixWriter - DecimalTag", "[ez][FixWriter]")
{
    char buffer[32] = {0};
    
    const char *target = "123=135.7" Delimiter;
    ez::FixWriter msg1(buffer);
    msg1.AddDecimal(123, 1357, -1);
    REQUIRE(memcmp(msg1.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg1.MsgSize() == strlen(target));
    REQUIRE(msg1.BufferSizeRemaining() == sizeof(buffer) - strlen(target));
    
    target = "123=-246.80" Delimiter;
    ez::FixWriter msg2(buffer);
    msg2.AddDecimal(123, -24680, -2);
    REQUIRE(memcmp(msg2.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg2.MsgSize() == strlen(target));
    REQUIRE(msg2.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    target = "123=543" Delimiter;
    ez::FixWriter msg3(buffer);
    msg3.AddDecimal(123, 543, 0);
    REQUIRE(memcmp(msg3.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg3.MsgSize() == strlen(target));
    REQUIRE(msg3.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    target = "123=111.3300" Delimiter;
    ez::FixWriter msg4(buffer);
    msg4.AddDecimal(123, 1113300, -4);
    REQUIRE(memcmp(msg4.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg4.MsgSize() == strlen(target));
    REQUIRE(msg4.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
    
    // tag as string literal
    ez::FixWriter msg5(buffer);
    msg5.AddDecimal("123", 1113300, -4);
    REQUIRE(memcmp(msg5.MsgBegin(), target, strlen(target)) == 0);
    REQUIRE(msg5.MsgSize() == strlen(target));
    REQUIRE(msg5.BufferSizeRemaining() == sizeof(buffer) - strlen(target)); 
}