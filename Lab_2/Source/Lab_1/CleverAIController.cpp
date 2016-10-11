// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "CleverAIController.h"

ACleverAIController::ACleverAIController()
{
    bDeliveringOrder = false;
    CurrentOrderNumber = -1;
}

float ACleverAIController::ComputeHouseScore(float Distance, float TimeLeft) const
{
    return - fmax(Distance - 150.f, 1.f) * (TimeLeft * TimeLeft);
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    if (bDeliveringOrder) {
        float Distance = GetDistanceToDestination(CurrentDestination);
        if (Distance < 300.f) {
            UE_LOG(LogTemp, Warning, TEXT("Trying to deliver order %d, current distance: %1.3f"), CurrentOrderNumber, Distance);
            bool bDeliveredOrder = TryDeliverPizza(CurrentOrderNumber);
            if (bDeliveredOrder) {
                UE_LOG(LogTemp, Warning, TEXT("Delivered order %d"), CurrentOrderNumber);
                bDeliveringOrder = false;
                CurrentOrderNumber = -1;
            }
        }
    }

    auto Orders = GetPizzaOrders();
    if (Orders.Num() == 0) {
        // No orders to serve.
        // TODO: Go to center of the map.
        return;
    }

    int PizzaAmount = GetPizzaAmount();
    if (PizzaAmount == 0) {
        bool bGrabbedPizza = TryGrabPizza();
        // Failed to retrieve pizza, need to get closer to the bakery.
        if (!bGrabbedPizza) {
            return;
        }
    }

    // Take first order.
    auto HouseLocations = GetHouseLocations();

    TSet<int> ServedOrders;
    for (int id = 0; id < GetControllerCount(); ++id) {
        if (id != ControllerId) {
            ACleverAIController* Controller = Cast<ACleverAIController>(GetControllerById(id));
            if (Controller) {
                int currentOrderNumber = Controller->GetCurrentOrderNumber();
                if (currentOrderNumber != -1) {
                    UE_LOG(LogTemp, Warning, TEXT("Controller %d is delivering order %d"), id, currentOrderNumber);
                    ServedOrders.Add(currentOrderNumber);
                }
            }
        }
    }

    int BestOrder = -1;
    float BestScore = 0;
    for (int i = 0; i < Orders.Num(); ++i) {
        if (ServedOrders.Contains(Orders[i].OrderNumber)) {
            continue;
        }
        int HouseNumber = Orders[i].HouseNumber;
        float Distance = GetDistanceToDestination(HouseLocations[HouseNumber]);
        float TimeLeft = GetHouseTimeLeft(HouseNumber);
        float Score = ComputeHouseScore(Distance, TimeLeft);

        if (BestOrder == -1 || Score > BestScore) {
            BestScore = Score;
            BestOrder = i;
        }
    }
    if (BestOrder == -1) {
        return;
    }

    auto Order = Orders[BestOrder];
    if (!bDeliveringOrder || Order.OrderNumber != CurrentOrderNumber) {
        auto HouseLocation = HouseLocations[Order.HouseNumber];
        bDeliveringOrder = true;
        CurrentOrderNumber = Order.OrderNumber;
        CurrentDestination = HouseLocation;
        SetNewMoveDestination(HouseLocation);
        UE_LOG(LogTemp, Warning, TEXT("Took new order %d to house %d with score %1.3f"),
            Order.OrderNumber,
            Order.HouseNumber,
            BestScore);
    }
}

int ACleverAIController::GetCurrentOrderNumber()
{
    return CurrentOrderNumber;
}
