#include "PriceUtil.hpp"

namespace ez {

int GetPriceRoundUp(int price100)
{
    if (price100 <= 1000)
    {
        price100 = ez::PriceRoundUp(price100, 1);
    }
    else if (price100 > 1000 && price100 <= 5000)
    {
        price100 = ez::PriceRoundUp(price100, 5);
    }
    else if (price100 > 5000 && price100 <= 10000)
    {
        price100 = ez::PriceRoundUp(price100, 10);
    }
    else if (price100 > 10000 && price100 <= 50000)
    {
        price100 = ez::PriceRoundUp(price100, 50);
    }
    else if (price100 > 50000 && price100 <= 100000)
    {
        price100 = ez::PriceRoundUp(price100, 100);
    }
    else
    {
        price100 = ez::PriceRoundUp(price100, 500);
    }
    
    return price100;
}

int GetPriceRoundDown(int price100)
{
    if (price100 < 1000)
    {
        price100 = ez::PriceRoundDown(price100, 1);
    }
    else if (price100 >= 1000 && price100 < 5000)
    {
        price100 = ez::PriceRoundDown(price100, 5);
    }
    else if (price100 >= 5000 && price100 < 10000)
    {
        price100 = ez::PriceRoundDown(price100, 10);
    }
    else if (price100 >= 10000 && price100 < 50000)
    {
        price100 = ez::PriceRoundDown(price100, 50);
    }
    else if (price100 >= 50000 && price100 < 100000)
    {
        price100 = ez::PriceRoundDown(price100, 100);
    }
    else
    {
        price100 = ez::PriceRoundDown(price100, 500);
    }
    
    return price100;
}

int GetPriceTickUp(int price100, int tickCount)
{
    for (int i = 0; i < tickCount; ++i)
    {
        if (price100 < 1000)
        {
            price100 = ez::PriceTickUp(price100, 1);
        }
        else if (price100 >= 1000 && price100 < 5000)
        {
            price100 = ez::PriceTickUp(price100, 5);
        }
        else if (price100 >= 5000 && price100 < 10000)
        {
            price100 = ez::PriceTickUp(price100, 10);
        }
        else if (price100 >= 10000 && price100 < 50000)
        {
            price100 = ez::PriceTickUp(price100, 50);
        }
        else if (price100 >= 50000 && price100 < 100000)
        {
            price100 = ez::PriceTickUp(price100, 100);
        }
        else
        {
            price100 = ez::PriceTickUp(price100, 500);
        }
    }

    return price100;
}

int GetPriceTickDown(int price100, int tickCount)
{
    for (int i = 0; i < tickCount; ++i) 
    {
        if (price100 <= 1000)
        {
            price100 = ez::PriceTickDown(price100, 1);
        }
        else if (price100 > 1000 && price100 <= 5000)
        {
            price100 = ez::PriceTickDown(price100, 5);
        }
        else if (price100 > 5000 && price100 <= 10000)
        {
            price100 = ez::PriceTickDown(price100, 10);
        }
        else if (price100 > 10000 && price100 <= 50000)
        {
            price100 = ez::PriceTickDown(price100, 50);
        }
        else if (price100 > 50000 && price100 <= 100000)
        {
            price100 = ez::PriceTickDown(price100, 100);
        }
        else
        {
            price100 = ez::PriceTickDown(price100, 500);
        }
    }

    return price100;
}

bool IsPriceOnTick(int price100)
{
    int priceModulo;
    
    if (price100 < 0)
        priceModulo = -1;
    else if (price100 <= 1000)
        priceModulo = 0;
    else if (price100 > 1000 && price100 <= 5000)
        priceModulo = price100 % 5;
    else if (price100 > 5000 && price100 <= 10000)
        priceModulo = price100 % 10;
    else if (price100 > 10000 && price100 <= 50000)
        priceModulo = price100 % 50;
    else if (price100 > 50000 && price100 <= 100000)
        priceModulo = price100 % 100;
    else
        priceModulo = price100 % 500;
    
    return (priceModulo == 0);
}

} // namespace ez
