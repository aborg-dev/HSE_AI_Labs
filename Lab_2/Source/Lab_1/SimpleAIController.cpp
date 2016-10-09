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
            UE_LOG(LogTemp, Warning, TEXT("Moving to order %d, current distance: %1.3f"), CurrentOrderNumber, Distance);
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

    // Take first not serverd order.
    auto HouseLocations = GetHouseLocations();

    TSet<int> ServedOrders;
    for (int id = 0; id < GetControllerCount(); ++id) {
        if (id != ControllerId) {
            ASimpleAIController* Controller = Cast<ASimpleAIController>(GetControllerById(id));
            if (Controller) {
                int currentOrderNumber = Controller->GetCurrentOrderNumber();
                if (currentOrderNumber != -1) {
                    UE_LOG(LogTemp, Warning, TEXT("Controller %d is delivering order %d"), id, currentOrderNumber);
                    ServedOrders.Add(currentOrderNumber);
                }
            }
        }
    }

    int closestOrder = -1;
    float closestDistance = 1e9;
    for (int i = 0; i < Orders.Num(); ++i) {
        if (ServedOrders.Contains(Orders[i].OrderNumber)) {
            continue;
        }
        float currentDistance = GetDistanceToDestination(HouseLocations[Orders[i].HouseNumber]);
        if (closestOrder == -1 || currentDistance < closestDistance) {
            closestDistance = currentDistance;
            closestOrder = i;
        }
    }
    if (closestOrder == -1) {
        return;
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

int ASimpleAIController::GetCurrentOrderNumber()
{
    return CurrentOrderNumber;
}
