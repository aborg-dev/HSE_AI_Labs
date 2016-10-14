// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "SimpleAIController.h"

ASimpleAIController::ASimpleAIController()
{
    bDeliveringOrder = false;
    CurrentOrderNumber = -1;
}

void ASimpleAIController::Tick(float DeltaSeconds)
{
    if (bDeliveringOrder) {
        float Distance = GetDistanceToDestination(CurrentDestination);
        if (Distance > 300.f) {
            SetNewMoveDestination(CurrentDestination);
            return;
        }
        UE_LOG(LogTemp, Warning, TEXT("Trying to deliver order %d, current distance: %1.3f"), CurrentOrderNumber, Distance);
        bool bDeliveredOrder = TryDeliverPizza(CurrentOrderNumber);
        if (bDeliveredOrder) {
            UE_LOG(LogTemp, Warning, TEXT("Delivered order %d"), CurrentOrderNumber);
            bDeliveringOrder = false;
            CurrentOrderNumber = -1;
        } else {
            SetNewMoveDestination(CurrentDestination);
        }
        return;
    }

    auto Orders = GetPizzaOrders();
    if (Orders.Num() == 0) {
        // No orders to serve.
        return;
    }

    // Take first order.
    auto HouseLocations = GetHouseLocations();

    int closestOrder = 0;
    int K = 45;
    float closestDistance = K*GetHouseTimeLeft(0) + GetDistanceToDestination(HouseLocations[Orders[0].HouseNumber]);
    for (int i = 0; i < Orders.Num(); ++i) {
        float currentDistance = K*GetHouseTimeLeft(i) + GetDistanceToDestination(HouseLocations[Orders[i].HouseNumber]);
        if (currentDistance < closestDistance) {
            closestDistance = currentDistance;
            closestOrder = i;
        }
    }
    auto Order = Orders[closestOrder];

    int PizzaAmount = GetPizzaAmount();
    if (PizzaAmount == 0) {
        bool bGrabbedPizza = TryGrabPizza();
        // Failed to retrieve pizza, need to get closer to the bakery.
        if (!bGrabbedPizza) {
            return;
        }
    }

    auto HouseLocation = HouseLocations[Order.HouseNumber];
    bDeliveringOrder = true;
    CurrentOrderNumber = Order.OrderNumber;
    CurrentDestination = HouseLocation;
    SetNewMoveDestination(HouseLocation);
    UE_LOG(LogTemp, Warning, TEXT("Took new order %d to house %d"), Order.OrderNumber, Order.HouseNumber);
}

