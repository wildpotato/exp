#ifndef __EZ_PRICE_UTIL_H__
#define __EZ_PRICE_UTIL_H__

#include <cmath>

namespace ez {

// 計算價差比率，以高價為100%
inline double GetPriceDiffRatio_HBase(long price1, long price2)
{
    if (price1 > 0 && price2 > 0)
    {
        long diff = (price1 > price2) ? (price1 - price2) : (price2 - price1);
        long base = (price1 > price2) ? price1 : price2;
        return ((double)diff / base) * 100;
    }
    else
        return 0;
}

// 計算價差比率，以低價為100%
inline double GetPriceDiffRatio_LBase(long price1, long price2)
{
    if (price1 > 0 && price2 > 0)
    {
        long diff = (price1 > price2) ? (price1 - price2) : (price2 - price1);
        long base = (price1 > price2) ? price2 : price1;
        return ((double)diff / base) * 100;
    }
    else
        return 0;
}
    
// 依百分比率取得反向報價的價格(不大於百分比), 以高價格做100%, 無條件向內捨去
// 用來計算交易所相關限制比率，交易所用高價當100%
inline long GetOpsQuotePrice_HIn(long price100, double range, char targetSide)
{
    double lowPriceRatio = (100 - range) / 100;

    if (lowPriceRatio <= 0)
        return price100;
    
    if (targetSide == '2')
        return (long)floor(price100 / lowPriceRatio);
    else
        return (long)ceil(price100 * lowPriceRatio);
}

// 依百分比率取得反向報價的價格(不小於百分比), 以高價格做100%, 無條件向外進位
// 用來計算交易所相關限制比率，交易所用高價當100%
inline long GetOpsQuotePrice_HOut(long price100, double range, char targetSide)
{
    double lowPriceRatio = (100 - range) / 100;

    if (lowPriceRatio <= 0)
        return price100;

    if (targetSide == '2')
        return (long)ceil(price100 / lowPriceRatio);
    else
        return (long)floor(price100 * lowPriceRatio);
}

// 依百分比率取得反向報價的價格(不大於百分比), 以低價格做100%, 無條件向內捨去
// 主要用來計算策略的利潤比率
inline long GetOpsQuotePrice_LIn(long price100, double range, char targetSide)
{
    double highPriceRatio = (100 + range) / 100;

    if (highPriceRatio <= 0)
        return price100;

    if (targetSide == '2')
        return (long)floor(price100 * highPriceRatio);
    else
        return (long)ceil(price100 / highPriceRatio);
}

// 依百分比率取得反向報價的價格(不小於百分比), 以低價格做100%, 無條件向外進位
// 主要用來計算策略的利潤比率
inline long GetOpsQuotePrice_LOut(long price100, double range, char targetSide)
{
    double highPriceRatio = (100 + range) / 100;

    if (highPriceRatio <= 0)
        return price100;

    if (targetSide == '2')
        return (long)ceil(price100 * highPriceRatio);
    else
        return (long)floor(price100 / highPriceRatio);
}

// 價格往上到最近的tick，如果本來已經在tick上，則價格不動
inline int PriceRoundUp(int price100, int tick) 
{
    int tmp = price100 + tick - 1;
    return tmp - (tmp % tick);
}

// 價格往下到最近的tick，如果本來已經在tick上，則價格不動
inline int PriceRoundDown(int price100, int tick) 
{
    return price100 - (price100 % tick);
}

// 價格往上到下一個tick，如果本來已經在tick上，則跳到下一個tick
inline int PriceTickUp(int price100, int tick)
{
    return ez::PriceRoundDown(price100, tick) + tick;
}

// 價格往下到下一個tick，如果本來已經在tick上，則跳到下一個tick
inline int PriceTickDown(int price100, int tick)
{
    return ez::PriceRoundUp(price100, tick) - tick;
}

// 依價格區間來跳到最近的tick
int GetPriceRoundUp(int price100);
int GetPriceRoundDown(int price100);

// 依價格區間來跳到下一個的tick
int GetPriceTickUp(int price100, int tickCount = 1);
int GetPriceTickDown(int price100, int tickCount = 1);

// 检查價格是否在tick上
bool IsPriceOnTick(int price100);

// 依百分比率取得反向報價的價格(不大於百分比), 以高價格做100%, 無條件向內捨去
// 用來計算交易所相關限制比率，交易所用高價當100%
inline long GetOpsPrice_HBase_Inside(long price100, double range, char targetSide)
{
    double lowPriceRatio = (100 - range) / 100;

    if (lowPriceRatio <= 0)
        return 0;
    
    if (targetSide == '2')
        return GetPriceRoundDown((int)floor(price100 / lowPriceRatio));
    else
        return GetPriceRoundUp((int)ceil(price100 * lowPriceRatio));
}

// 依百分比率取得反向報價的價格(不小於百分比), 以高價格做100%, 無條件向外進位
// 用來計算交易所相關限制比率，交易所用高價當100%
inline long GetOpsPrice_HBase_Outside(long price100, double range, char targetSide)
{
    double lowPriceRatio = (100 - range) / 100;

    if (lowPriceRatio <= 0)
        return 0;

    if (targetSide == '2')
        return GetPriceRoundUp((int)ceil(price100 / lowPriceRatio));
    else
        return GetPriceRoundDown((int)floor(price100 * lowPriceRatio));
}

// 依百分比率取得反向報價的價格(不大於百分比), 以低價格做100%, 無條件向內捨去
// 主要用來計算策略的利潤比率
inline long GetOpsPrice_LBase_Inside(long price100, double range, char targetSide)
{
    double highPriceRatio = (100 + range) / 100;

    if (highPriceRatio <= 0)
        return 0;

    if (targetSide == '2')
        return GetPriceRoundDown((int)floor(price100 * highPriceRatio));
    else
        return GetPriceRoundUp((int)ceil(price100 / highPriceRatio));
}

// 依百分比率取得反向報價的價格(不小於百分比), 以低價格做100%, 無條件向外進位
// 主要用來計算策略的利潤比率
inline long GetOpsPrice_LBase_Outside(long price100, double range, char targetSide)
{
    double highPriceRatio = (100 + range) / 100;

    if (highPriceRatio <= 0)
        return 0;

    if (targetSide == '2')
        return GetPriceRoundUp((int)ceil(price100 * highPriceRatio));
    else
        return GetPriceRoundDown((int)floor(price100 / highPriceRatio));
}

// 用來取得報價變動後的極限價格，報價的價格跳動比率是用舊價格當基準100%
inline long GetQuoteLowLimit_Inside(long price100, double range)
{
    double ratio = (100 - range) / 100;
    return GetPriceRoundUp((int)ceil(price100 * ratio));
}
inline long GetQuoteHighLimit_Inside(long price100, double range)
{
    double ratio = (100 + range) / 100;
    return GetPriceRoundDown((int)floor(price100 * ratio));
}


} // namespace ez

#endif // __EZ_PRICE_UTIL_H__