#include "catch.hpp"
#include "PriceUtil.hpp"

using namespace ez;

TEST_CASE("GetPriceDiffRatio_HBase", "[ez][PriceUtil]")
{
    // (102 - 100) / 102 = 0.01960784313
    REQUIRE(GetPriceDiffRatio_HBase(102, 100) == Approx(1.960784313));
    REQUIRE(GetPriceDiffRatio_HBase(100, 102) == Approx(1.960784313));
    
    // (53 - 51) / 53 = 0.03773584905
    REQUIRE(GetPriceDiffRatio_HBase(53, 51) == Approx(3.773584905));
    REQUIRE(GetPriceDiffRatio_HBase(51, 53) == Approx(3.773584905));
}

TEST_CASE("GetPriceDiffRatio_LBase", "[ez][PriceUtil]")
{
    // (102 - 100) / 100 = 0.02
    REQUIRE(GetPriceDiffRatio_LBase(102, 100) == Approx(2));
    REQUIRE(GetPriceDiffRatio_LBase(100, 102) == Approx(2));
    
    // (53 - 51) / 51 = 0.03921568627
    REQUIRE(GetPriceDiffRatio_LBase(53, 51) == Approx(3.921568627));
    REQUIRE(GetPriceDiffRatio_LBase(51, 53) == Approx(3.921568627));    
}

TEST_CASE("PriceRoundUp", "[ez][PriceUtil]")
{
    REQUIRE(PriceRoundUp(0, 10) == 0);
    REQUIRE(PriceRoundUp(1, 10) == 10);
    
    REQUIRE(PriceRoundUp(98, 10) == 100);
    REQUIRE(PriceRoundUp(99, 10) == 100);
    REQUIRE(PriceRoundUp(100, 10) == 100);
    REQUIRE(PriceRoundUp(101, 10) == 110);
    REQUIRE(PriceRoundUp(102, 10) == 110);
    
    REQUIRE(PriceRoundUp(98, 5) == 100);
    REQUIRE(PriceRoundUp(99, 5) == 100);
    REQUIRE(PriceRoundUp(100, 5) == 100);
    REQUIRE(PriceRoundUp(101, 5) == 105);
    REQUIRE(PriceRoundUp(102, 5) == 105);
}

TEST_CASE("PriceRoundDown", "[ez][PriceUtil]")
{
    REQUIRE(PriceRoundDown(0, 10) == 0);
    REQUIRE(PriceRoundDown(1, 10) == 0);
    
    REQUIRE(PriceRoundDown(98, 10) == 90);
    REQUIRE(PriceRoundDown(99, 10) == 90);
    REQUIRE(PriceRoundDown(100, 10) == 100);
    REQUIRE(PriceRoundDown(101, 10) == 100);
    REQUIRE(PriceRoundDown(102, 10) == 100);
    
    REQUIRE(PriceRoundDown(98, 5) == 95);
    REQUIRE(PriceRoundDown(99, 5) == 95);
    REQUIRE(PriceRoundDown(100, 5) == 100);
    REQUIRE(PriceRoundDown(101, 5) == 100);
    REQUIRE(PriceRoundDown(102, 5) == 100);
}

TEST_CASE("PriceTickUp", "[ez][PriceUtil]")
{
    REQUIRE(PriceTickUp(0, 10) == 10);
    REQUIRE(PriceTickUp(1, 10) == 10);
    
    REQUIRE(PriceTickUp(98, 10) == 100);
    REQUIRE(PriceTickUp(99, 10) == 100);
    REQUIRE(PriceTickUp(100, 10) == 110);
    REQUIRE(PriceTickUp(101, 10) == 110);
    REQUIRE(PriceTickUp(102, 10) == 110);
    
    REQUIRE(PriceTickUp(98, 5) == 100);
    REQUIRE(PriceTickUp(99, 5) == 100);
    REQUIRE(PriceTickUp(100, 5) == 105);
    REQUIRE(PriceTickUp(101, 5) == 105);
    REQUIRE(PriceTickUp(102, 5) == 105);
}

TEST_CASE("PriceTickDown", "[ez][PriceUtil]")
{
    REQUIRE(PriceTickDown(0, 10) == -10);
    REQUIRE(PriceTickDown(1, 10) == 0);
    
    REQUIRE(PriceTickDown(98, 10) == 90);
    REQUIRE(PriceTickDown(99, 10) == 90);
    REQUIRE(PriceTickDown(100, 10) == 90);
    REQUIRE(PriceTickDown(101, 10) == 100);
    REQUIRE(PriceTickDown(102, 10) == 100);
    
    REQUIRE(PriceTickDown(98, 5) == 95);
    REQUIRE(PriceTickDown(99, 5) == 95);
    REQUIRE(PriceTickDown(100, 5) == 95);
    REQUIRE(PriceTickDown(101, 5) == 100);
    REQUIRE(PriceTickDown(102, 5) == 100);
}

TEST_CASE("GetPriceRoundUp", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceRoundUp(998) == 998);
    REQUIRE(GetPriceRoundUp(999) == 999);
    REQUIRE(GetPriceRoundUp(1000) == 1000);
    REQUIRE(GetPriceRoundUp(1001) == 1005);
    REQUIRE(GetPriceRoundUp(1004) == 1005);
    REQUIRE(GetPriceRoundUp(1005) == 1005);
    
    REQUIRE(GetPriceRoundUp(4995) == 4995);
    REQUIRE(GetPriceRoundUp(4999) == 5000);
    REQUIRE(GetPriceRoundUp(5000) == 5000);
    REQUIRE(GetPriceRoundUp(5001) == 5010);
    REQUIRE(GetPriceRoundUp(5009) == 5010);
    REQUIRE(GetPriceRoundUp(5010) == 5010);
    
    REQUIRE(GetPriceRoundUp(9990) == 9990);
    REQUIRE(GetPriceRoundUp(9999) == 10000);
    REQUIRE(GetPriceRoundUp(10000) == 10000);
    REQUIRE(GetPriceRoundUp(10001) == 10050);
    REQUIRE(GetPriceRoundUp(10049) == 10050);
    REQUIRE(GetPriceRoundUp(10050) == 10050);
    
    REQUIRE(GetPriceRoundUp(49950) == 49950);
    REQUIRE(GetPriceRoundUp(49999) == 50000);
    REQUIRE(GetPriceRoundUp(50000) == 50000);
    REQUIRE(GetPriceRoundUp(50001) == 50100);
    REQUIRE(GetPriceRoundUp(50099) == 50100);
    REQUIRE(GetPriceRoundUp(50100) == 50100);
    
    REQUIRE(GetPriceRoundUp(99900) == 99900);
    REQUIRE(GetPriceRoundUp(99999) == 100000);
    REQUIRE(GetPriceRoundUp(100000) == 100000);
    REQUIRE(GetPriceRoundUp(100001) == 100500);
    REQUIRE(GetPriceRoundUp(100499) == 100500);
    REQUIRE(GetPriceRoundUp(100500) == 100500);
}

TEST_CASE("GetPriceRoundDown", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceRoundDown(998) == 998);
    REQUIRE(GetPriceRoundDown(999) == 999);
    REQUIRE(GetPriceRoundDown(1000) == 1000);
    REQUIRE(GetPriceRoundDown(1001) == 1000);
    REQUIRE(GetPriceRoundDown(1005) == 1005);
    
    REQUIRE(GetPriceRoundDown(4995) == 4995);
    REQUIRE(GetPriceRoundDown(4996) == 4995);
    REQUIRE(GetPriceRoundDown(4999) == 4995);
    REQUIRE(GetPriceRoundDown(5000) == 5000);
    REQUIRE(GetPriceRoundDown(5001) == 5000);
    REQUIRE(GetPriceRoundDown(5010) == 5010);
    
    REQUIRE(GetPriceRoundDown(9990) == 9990);
    REQUIRE(GetPriceRoundDown(9991) == 9990);
    REQUIRE(GetPriceRoundDown(9999) == 9990);
    REQUIRE(GetPriceRoundDown(10000) == 10000);
    REQUIRE(GetPriceRoundDown(10001) == 10000);
    REQUIRE(GetPriceRoundDown(10050) == 10050);
    
    REQUIRE(GetPriceRoundDown(49950) == 49950);
    REQUIRE(GetPriceRoundDown(49951) == 49950);
    REQUIRE(GetPriceRoundDown(49999) == 49950);
    REQUIRE(GetPriceRoundDown(50000) == 50000);
    REQUIRE(GetPriceRoundDown(50001) == 50000);
    REQUIRE(GetPriceRoundDown(50100) == 50100);
    
    REQUIRE(GetPriceRoundDown(99900) == 99900);
    REQUIRE(GetPriceRoundDown(99901) == 99900);
    REQUIRE(GetPriceRoundDown(99999) == 99900);
    REQUIRE(GetPriceRoundDown(100000) == 100000);
    REQUIRE(GetPriceRoundDown(100001) == 100000);
    REQUIRE(GetPriceRoundDown(100500) == 100500);
}

TEST_CASE("GetPriceTickUp", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceTickUp(998) == 999);
    REQUIRE(GetPriceTickUp(999) == 1000);
    REQUIRE(GetPriceTickUp(1000) == 1005);
    REQUIRE(GetPriceTickUp(1001) == 1005);
    REQUIRE(GetPriceTickUp(1004) == 1005);
    REQUIRE(GetPriceTickUp(1005) == 1010);
    
    REQUIRE(GetPriceTickUp(4995) == 5000);
    REQUIRE(GetPriceTickUp(4999) == 5000);
    REQUIRE(GetPriceTickUp(5000) == 5010);
    REQUIRE(GetPriceTickUp(5001) == 5010);
    REQUIRE(GetPriceTickUp(5009) == 5010);
    REQUIRE(GetPriceTickUp(5010) == 5020);
    
    REQUIRE(GetPriceTickUp(9990) == 10000);
    REQUIRE(GetPriceTickUp(9999) == 10000);
    REQUIRE(GetPriceTickUp(10000) == 10050);
    REQUIRE(GetPriceTickUp(10001) == 10050);
    REQUIRE(GetPriceTickUp(10049) == 10050);
    REQUIRE(GetPriceTickUp(10050) == 10100);
    
    REQUIRE(GetPriceTickUp(49950) == 50000);
    REQUIRE(GetPriceTickUp(49999) == 50000);
    REQUIRE(GetPriceTickUp(50000) == 50100);
    REQUIRE(GetPriceTickUp(50001) == 50100);
    REQUIRE(GetPriceTickUp(50099) == 50100);
    REQUIRE(GetPriceTickUp(50100) == 50200);
    
    REQUIRE(GetPriceTickUp(99900) == 100000);
    REQUIRE(GetPriceTickUp(99999) == 100000);
    REQUIRE(GetPriceTickUp(100000) == 100500);
    REQUIRE(GetPriceTickUp(100001) == 100500);
    REQUIRE(GetPriceTickUp(100499) == 100500);
    REQUIRE(GetPriceTickUp(100500) == 101000);
}

TEST_CASE("GetPriceTickUp - multi tick count", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceTickUp(999) == 1000);
    REQUIRE(GetPriceTickUp(999, 2) == 1005);
    REQUIRE(GetPriceTickUp(999, 3) == 1010);
    
    REQUIRE(GetPriceTickUp(1000) == 1005);
    REQUIRE(GetPriceTickUp(1000, 2) == 1010);
    REQUIRE(GetPriceTickUp(1000, 3) == 1015);
    
    REQUIRE(GetPriceTickUp(1001) == 1005);
    REQUIRE(GetPriceTickUp(1001, 2) == 1010);
    REQUIRE(GetPriceTickUp(1001, 3) == 1015);
}

TEST_CASE("GetPriceTickDown", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceTickDown(998) == 997);
    REQUIRE(GetPriceTickDown(999) == 998);
    REQUIRE(GetPriceTickDown(1000) == 999);
    REQUIRE(GetPriceTickDown(1001) == 1000);
    REQUIRE(GetPriceTickDown(1005) == 1000);
    
    REQUIRE(GetPriceTickDown(4995) == 4990);
    REQUIRE(GetPriceTickDown(4996) == 4995);
    REQUIRE(GetPriceTickDown(4999) == 4995);
    REQUIRE(GetPriceTickDown(5000) == 4995);
    REQUIRE(GetPriceTickDown(5001) == 5000);
    REQUIRE(GetPriceTickDown(5010) == 5000);
    
    REQUIRE(GetPriceTickDown(9990) == 9980);
    REQUIRE(GetPriceTickDown(9991) == 9990);
    REQUIRE(GetPriceTickDown(9999) == 9990);
    REQUIRE(GetPriceTickDown(10000) == 9990);
    REQUIRE(GetPriceTickDown(10001) == 10000);
    REQUIRE(GetPriceTickDown(10050) == 10000);
    
    REQUIRE(GetPriceTickDown(49950) == 49900);
    REQUIRE(GetPriceTickDown(49951) == 49950);
    REQUIRE(GetPriceTickDown(49999) == 49950);
    REQUIRE(GetPriceTickDown(50000) == 49950);
    REQUIRE(GetPriceTickDown(50001) == 50000);
    REQUIRE(GetPriceTickDown(50100) == 50000);
    
    REQUIRE(GetPriceTickDown(99900) == 99800);
    REQUIRE(GetPriceTickDown(99901) == 99900);
    REQUIRE(GetPriceTickDown(99999) == 99900);
    REQUIRE(GetPriceTickDown(100000) == 99900);
    REQUIRE(GetPriceTickDown(100001) == 100000);
    REQUIRE(GetPriceTickDown(100500) == 100000);
}

TEST_CASE("GetPriceTickDown - multi tick count", "[ez][PriceUtil]")
{
    REQUIRE(GetPriceTickDown(4999) == 4995);
    REQUIRE(GetPriceTickDown(4999, 2) == 4990);
    REQUIRE(GetPriceTickDown(4999, 3) == 4985);
    
    REQUIRE(GetPriceTickDown(5000) == 4995);
    REQUIRE(GetPriceTickDown(5000, 2) == 4990);
    REQUIRE(GetPriceTickDown(5000, 3) == 4985);
    
    REQUIRE(GetPriceTickDown(5001) == 5000);
    REQUIRE(GetPriceTickDown(5001, 2) == 4995);
    REQUIRE(GetPriceTickDown(5001, 3) == 4990);
}

TEST_CASE("IsPriceOnTick", "[ez][PriceUtil]")
{
    REQUIRE(IsPriceOnTick(998) == true);
    REQUIRE(IsPriceOnTick(999) == true);
    REQUIRE(IsPriceOnTick(1000) == true);
    REQUIRE(IsPriceOnTick(1001) == false);
    REQUIRE(IsPriceOnTick(1005) == true);
    
    REQUIRE(IsPriceOnTick(4995) == true);
    REQUIRE(IsPriceOnTick(4996) == false);
    REQUIRE(IsPriceOnTick(4999) == false);
    REQUIRE(IsPriceOnTick(5000) == true);
    REQUIRE(IsPriceOnTick(5001) == false);
    REQUIRE(IsPriceOnTick(5010) == true);
    
    REQUIRE(IsPriceOnTick(9990) == true);
    REQUIRE(IsPriceOnTick(9991) == false);
    REQUIRE(IsPriceOnTick(9999) == false);
    REQUIRE(IsPriceOnTick(10000) == true);
    REQUIRE(IsPriceOnTick(10001) == false);
    REQUIRE(IsPriceOnTick(10050) == true);
    
    REQUIRE(IsPriceOnTick(49950) == true);
    REQUIRE(IsPriceOnTick(49951) == false);
    REQUIRE(IsPriceOnTick(49999) == false);
    REQUIRE(IsPriceOnTick(50000) == true);
    REQUIRE(IsPriceOnTick(50001) == false);
    REQUIRE(IsPriceOnTick(50100) == true);
    
    REQUIRE(IsPriceOnTick(99900) == true);
    REQUIRE(IsPriceOnTick(99901) == false);
    REQUIRE(IsPriceOnTick(99999) == false);
    REQUIRE(IsPriceOnTick(100000) == true);
    REQUIRE(IsPriceOnTick(100001) == false);
    REQUIRE(IsPriceOnTick(100500) == true);
}

TEST_CASE("GetOpsPrice_HBase_Inside - get price", "[ez][PriceUtil]")
{
    // 50 * 0.9968 = 49.84
    REQUIRE(GetOpsPrice_HBase_Inside(5000, 0.32, '1') == 4985);
    // 50 / 0.9968 = 50.1605136437
    REQUIRE(GetOpsPrice_HBase_Inside(5000, 0.32, '2') == 5010);

    // 87.87 * 0.9968 = 87.588816
    REQUIRE(GetOpsPrice_HBase_Inside(8787, 0.32, '1') == 8760);    
    // 87.87 / 0.9968 = 88.1520866774
    REQUIRE(GetOpsPrice_HBase_Inside(8787, 0.32, '2') == 8810);

    // 123.32 * 0.9968 = 122.925376
    REQUIRE(GetOpsPrice_HBase_Inside(12332, 0.32, '1') == 12300);
    // 123.32 / 0.9968 = 123.715890851
    REQUIRE(GetOpsPrice_HBase_Inside(12332, 0.32, '2') == 12350);    
}

TEST_CASE("GetOpsPrice_HBase_Outside - get price", "[ez][PriceUtil]")
{
    // 50 * 0.9968 = 49.84
    REQUIRE(GetOpsPrice_HBase_Outside(5000, 0.32, '1') == 4980);
    // 50 / 0.9968 = 50.1605136437
    REQUIRE(GetOpsPrice_HBase_Outside(5000, 0.32, '2') == 5020);

    // 87.87 * 0.9968 = 87.588816
    REQUIRE(GetOpsPrice_HBase_Outside(8787, 0.32, '1') == 8750);    
    // 87.87 / 0.9968 = 88.1520866774
    REQUIRE(GetOpsPrice_HBase_Outside(8787, 0.32, '2') == 8820);

    // 123.32 * 0.9968 = 122.925376
    REQUIRE(GetOpsPrice_HBase_Outside(12332, 0.32, '1') == 12250);
    // 123.32 / 0.9968 = 123.715890851
    REQUIRE(GetOpsPrice_HBase_Outside(12332, 0.32, '2') == 12400);    
}

TEST_CASE("GetOpsPrice_LBase_Inside - get price", "[ez][PriceUtil]")
{
    // 50 / 1.0032 = 49.8405103668
    REQUIRE(GetOpsPrice_LBase_Inside(5000, 0.32, '1') == 4985);
    // 50 * 1.0032 = 50.16
    REQUIRE(GetOpsPrice_LBase_Inside(5000, 0.32, '2') == 5010);

    // 87.87 / 1.0032 = 87.5897129187
    REQUIRE(GetOpsPrice_LBase_Inside(8787, 0.32, '1') == 8760);    
    // 87.87 * 1.0032 = 88.151184
    REQUIRE(GetOpsPrice_LBase_Inside(8787, 0.32, '2') == 8810);

    // 123.32 / 1.0032 = 122.926634769
    REQUIRE(GetOpsPrice_LBase_Inside(12332, 0.32, '1') == 12300);
    // 123.32 * 1.0032 = 123.714624
    REQUIRE(GetOpsPrice_LBase_Inside(12332, 0.32, '2') == 12350);    
}

TEST_CASE("GetOpsPrice_LBase_Outside - get price", "[ez][PriceUtil]")
{
    // 50 / 1.0032 = 49.8405103668
    REQUIRE(GetOpsPrice_LBase_Outside(5000, 0.32, '1') == 4980);
    // 50 * 1.0032 = 50.16
    REQUIRE(GetOpsPrice_LBase_Outside(5000, 0.32, '2') == 5020);

    // 87.87 / 1.0032 = 87.5897129187
    REQUIRE(GetOpsPrice_LBase_Outside(8787, 0.32, '1') == 8750);    
    // 87.87 * 1.0032 = 88.151184
    REQUIRE(GetOpsPrice_LBase_Outside(8787, 0.32, '2') == 8820);

    // 123.32 / 1.0032 = 122.926634769
    REQUIRE(GetOpsPrice_LBase_Outside(12332, 0.32, '1') == 12250);
    // 123.32 * 1.0032 = 123.714624
    REQUIRE(GetOpsPrice_LBase_Outside(12332, 0.32, '2') == 12400);    
}

TEST_CASE("GetQuoteLowLimit_Inside", "[ez][PriceUtil]")
{
    // 50 * 0.92 = 46
    REQUIRE(GetQuoteLowLimit_Inside(5000, 8) == 4600);
    
    // 87.87 * 0.92 = 80.8404
    REQUIRE(GetQuoteLowLimit_Inside(8787, 8) == 8090);
    
    // 123.32 * 0.92 = 113.4544
    REQUIRE(GetQuoteLowLimit_Inside(12332, 8) == 11350);
}

TEST_CASE("GetQuoteHighLimit_Inside", "[ez][PriceUtil]")
{
    // 50 * 1.08 = 54
    REQUIRE(GetQuoteHighLimit_Inside(5000, 8) == 5400);
    
    // 87.87 * 1.08 = 94.8996
    REQUIRE(GetQuoteHighLimit_Inside(8787, 8) == 9480);
    
    // 123.32 * 1.08 = 133.1856
    REQUIRE(GetQuoteHighLimit_Inside(12332, 8) == 13300);    
}