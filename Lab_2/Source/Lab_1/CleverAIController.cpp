// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "CleverAIController.h"

ACleverAIController::ACleverAIController()
{
    bDeliveringOrder = false;
    CurrentOrderNumber = -1;
    PendingOrderNumber = -1;
    MasterDecisionTimer = 0.0f;
}

float ACleverAIController::ComputeHouseScore(float Distance, float TimeLeft) const
{
    return - fmax(Distance - 150.f, 1.f) * (TimeLeft * TimeLeft);
}

struct ControllerOrderServeTime
{
    ControllerOrderServeTime(float time, int orderIndex, int controllerId)
        : Time(time)
        , OrderIndex(orderIndex)
        , ControllerId(controllerId)
    { }

    float Time;

    int OrderIndex;
    int ControllerId;
};

bool operator < (const ControllerOrderServeTime& lhs, const ControllerOrderServeTime& rhs)
{
    return lhs.Time < rhs.Time;
}

float ACleverAIController::GetTimeToCoverDistance(float distance)
{
    return distance / GetCharacterMaxSpeed();
}

void ACleverAIController::ActAsMaster()
{
    auto orders = GetPizzaOrders();
    if (orders.Num() == 0) {
        // No orders to serve.
        // TODO: Go to center of the map.
        return;
    }

    auto houseLocations = GetHouseLocations();

    TArray<ACleverAIController*> controllers;
    for (int id = 0; id < GetControllerCount(); ++id) {
        controllers.Add(Cast<ACleverAIController>(GetControllerById(id)));
    }

    TArray<FVector> controllerLocations;
    for (auto* controller : controllers) {
        controllerLocations.Add(controller->GetCharacterLocation());
    }

    TArray<ControllerOrderServeTime> controllerOrderServeTimes;
    for (int controllerId = 0; controllerId < controllerLocations.Num(); ++controllerId) {
        for (int orderIndex = 0; orderIndex < orders.Num(); ++orderIndex) {
            auto& controllerLocation = controllerLocations[controllerId];
            int houseNumber = orders[orderIndex].HouseNumber;
            auto& orderLocation = houseLocations[houseNumber];
            auto distance = GetDistanceBetween(controllerLocation, orderLocation);
            if (distance < 300.0f) {
                distance = -1e6;
            }
            auto time = GetTimeToCoverDistance(distance) - orders[orderIndex].CurrentWaitTime * orders[orderIndex].CurrentWaitTime;
            controllerOrderServeTimes.Add(ControllerOrderServeTime(time, orderIndex, controllerId));
        }
    }
    controllerOrderServeTimes.Sort();

    TArray<FVector> currentControllerLocations = controllerLocations;
    TArray<float> currentControllerTravelTime;
    currentControllerTravelTime.SetNumZeroed(controllers.Num());
    TArray<TArray<int>> assignedControllerOrders;
    assignedControllerOrders.SetNumZeroed(controllers.Num());

    int assignedControllers = 0;

    TSet<int> ServedOrders;
    for (const auto& controllerOrderServeTime : controllerOrderServeTimes) {
        auto orderIndex = controllerOrderServeTime.OrderIndex;
        auto controllerId = controllerOrderServeTime.ControllerId;

        if (ServedOrders.Contains(orderIndex)) {
            continue;
        }

        const auto& orderLocation = houseLocations[orders[orderIndex].HouseNumber];
        auto distance = GetDistanceBetween(currentControllerLocations[controllerId], orderLocation);
        auto time = GetTimeToCoverDistance(distance);
        auto nextControllerTime = currentControllerTravelTime[controllerId] + time;

        UE_LOG(LogTemp, Warning, TEXT("%d -> %d: %.3f"), controllerId, orders[orderIndex].OrderNumber, distance);

        if (assignedControllerOrders[controllerId].Num() > 0 && nextControllerTime > 10) {
            continue;
        }
        assignedControllerOrders[controllerId].Add(orderIndex);
        currentControllerTravelTime[controllerId] = nextControllerTime;
        currentControllerLocations[controllerId] = orderLocation;
        ServedOrders.Add(orderIndex);

        if (assignedControllerOrders[controllerId].Num() == 1) {
            ++assignedControllers;
        }
        if (assignedControllers == controllers.Num()) {
            break;
        }
    }

    for (int controllerId = 0; controllerId < controllers.Num(); ++controllerId) {
        const auto& controllerOrders = assignedControllerOrders[controllerId];
        if (controllerOrders.Num() > 0) {
            controllers[controllerId]->SetPendingOrderNumber(orders[controllerOrders[0]].OrderNumber);
        }
    }
}

FVector ACleverAIController::GetHouseLocation(int HouseNumber)
{
    auto houseLocations = GetHouseLocations();
    return houseLocations[HouseNumber];
}

int FindOrderIndex(const TArray<FPizzaOrder>& orders, int OrderNumber)
{
    for (int i = 0; i < orders.Num(); ++i) {
        if (orders[i].OrderNumber == OrderNumber) {
            return i;
        }
    }
    return -1;
}

void ACleverAIController::ActAsWorker()
{
    if (PendingOrderNumber != -1 && (!bDeliveringOrder || PendingOrderNumber != CurrentOrderNumber)) {
        auto orders = GetPizzaOrders();
        int OrderIndex = FindOrderIndex(orders, PendingOrderNumber);
        if (OrderIndex == -1) {
            return;
        }
        auto& Order = orders[OrderIndex];
        auto HouseLocation = GetHouseLocation(Order.HouseNumber);
        bDeliveringOrder = true;
        CurrentOrderNumber = Order.OrderNumber;
        CurrentDestination = HouseLocation;
        PendingOrderNumber = -1;
        SetNewMoveDestination(HouseLocation);
        UE_LOG(LogTemp, Warning, TEXT("Controller %d took new order %d to house %d"),
            ControllerId,
            Order.OrderNumber,
            Order.HouseNumber);
    }
}

void ACleverAIController::SetPendingOrderNumber(int OrderNumber)
{
    PendingOrderNumber = OrderNumber;
}

void ACleverAIController::TryDeliverOrder()
{
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

void ACleverAIController::RefillPizzaIfNeeded()
{
    int PizzaAmount = GetPizzaAmount();
    if (PizzaAmount == 0) {
        bool bGrabbedPizza = TryGrabPizza();
    }
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    if (bDeliveringOrder) {
        TryDeliverOrder();
    }
    RefillPizzaIfNeeded();

    if (ControllerId == MASTER_ID) {
        MasterDecisionTimer += DeltaSeconds;
        if (MasterDecisionTimer >= MASTER_DECISION_PERIOD) {
            ActAsMaster();
            MasterDecisionTimer -= MASTER_DECISION_PERIOD;
        }
    }
    ActAsWorker();
}

int ACleverAIController::GetCurrentOrderNumber()
{
    return CurrentOrderNumber;
}
