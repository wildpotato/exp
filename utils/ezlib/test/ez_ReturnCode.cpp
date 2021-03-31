#include "catch.hpp"
#include "ReturnCode.hpp"

using ReturnCode = ez::ReturnCode;

TEST_CASE("ReturnCode construction", "[ez][ReturnCode]")
{
    SECTION("ok")
    {
        ReturnCode rcOK;
        REQUIRE(rcOK.IsOK() == true);
        REQUIRE(rcOK.IsFail() == false);
        REQUIRE(rcOK.GetCode() == 0);
        REQUIRE(rcOK.GetStatus() == 0);
    }

    SECTION("fail")
    {
        ReturnCode rcFail(-1, -2);
        REQUIRE(rcFail.IsOK() == false);
        REQUIRE(rcFail.IsFail() == true);
        REQUIRE(rcFail.GetCode() == -1);
        REQUIRE(rcFail.GetStatus() == -2);
    }
}

TEST_CASE("ReturnCode conversion to bool", "[ez][ReturnCode]")
{
    SECTION( "ok" )
    {
        ReturnCode rcOK;
        bool isOK(rcOK);
        REQUIRE(isOK == true);
    }

    SECTION( "fail" )
    {
        ReturnCode rcFail(-1, -2);
        bool isOK(rcFail);
        REQUIRE(isOK == false);
    }
}