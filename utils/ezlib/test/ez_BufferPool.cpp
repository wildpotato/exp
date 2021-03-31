#include "catch.hpp"
#include "BufferPool.hpp"

TEST_CASE("Buffer - default ctor", "[ez][Buffer]")
{
    ez::Buffer buffer;
    
    REQUIRE((bool)buffer == false);
    REQUIRE(buffer.IsValid() == false);
    
    REQUIRE(buffer.Begin() == nullptr);
    REQUIRE(buffer.End() == nullptr);
    
    REQUIRE(buffer.GetMaxSize() == 0);
}

TEST_CASE("BufferPool - get fixed size buffer", "[ez][BufferPool]")
{
    ez::Buffer buffer = ez::Get128Buffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 128);
    
    buffer = ez::Get256Buffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 256);
    
    buffer = ez::Get512Buffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 512);
    
    buffer = ez::Get1KBuffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 1024);
    
    buffer = ez::Get2KBuffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 2048);
    
    buffer = ez::Get4KBuffer();
    REQUIRE(buffer.IsValid() == true);
    REQUIRE(buffer.Begin() != nullptr);
    REQUIRE(buffer.GetMaxSize() == 4096);
}

TEST_CASE("BufferPool - get dynamic size buffer", "[ez][BufferPool]")
{
    ez::Buffer buffer = ez::GetBuffer(135);
    REQUIRE(buffer.GetMaxSize() == 135);
    
    buffer = ez::GetBuffer(246);
    REQUIRE(buffer.GetMaxSize() == 246);
}

TEST_CASE("BufferPool - get align buffer", "[ez][BufferPool]")
{
    ez::Buffer buffer = ez::GetBuffer_Align(127);
    REQUIRE(buffer.GetMaxSize() == 128);
    
    buffer = ez::GetBuffer_Align(128);
    REQUIRE(buffer.GetMaxSize() == 128);
    
    buffer = ez::GetBuffer_Align(129);
    REQUIRE(buffer.GetMaxSize() == 256);
}