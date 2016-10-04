#pragma once

struct FPizzaOrder
{
    FPizzaOrder(int orderNumber, int houseNumber, int amount)
        : OrderNumber(orderNumber)
        , HouseNumber(houseNumber)
        , Amount(amount)
        , CurrentWaitTime(0.f)
    { }

    int OrderNumber;
    int HouseNumber;
    int Amount;
    float CurrentWaitTime;
};

