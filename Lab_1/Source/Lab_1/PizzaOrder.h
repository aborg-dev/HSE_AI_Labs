#pragma once

struct FPizzaOrder
{
    FPizzaOrder(int orderNumber, int houseNumber, int amount)
        : OrderNumber(orderNumber)
        , HouseNumber(houseNumber)
        , Amount(amount) {}

    int OrderNumber;
    int HouseNumber;
    int Amount;
};

