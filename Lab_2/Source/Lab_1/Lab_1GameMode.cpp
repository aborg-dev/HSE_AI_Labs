// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_1.h"
#include "Lab_1GameMode.h"
#include "Lab_1PlayerController.h"
#include "Lab_1Character.h"
#include "PizzaHUD.h"
#include "HouseActor.h"

#include <vector>
#include <algorithm>

ALab_1GameMode::ALab_1GameMode()
{
    // Set the default HUD class to be used in game.
    HUDClass = APizzaHUD::StaticClass();

    // Spawn pizza every 2 seconds.
    SpawnDelay = 2.0f;

    TotalPizzaOrderCount = 0;
    DeliveredPizzaOrderCount = 0;
    DeliveredPizzaWaitTime = 0.0f;
}

void ALab_1GameMode::BeginPlay()
{
    // Don't forget to call parent BeginPlay() method.
    Super::BeginPlay();

    DiscoverHouses();

    SpawnPizzaTimer = 0.0f;
    RandomStream.Initialize(42);
    // Transition the game into playing state.
    SetCurrentState(ELab_1PlayState::EPlaying);
}

void ALab_1GameMode::DiscoverHouses()
{
    // Find all House actors.
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHouseActor::StaticClass(), FoundActors);
    for (auto Actor : FoundActors) {
        AHouseActor* HouseActor = Cast<AHouseActor>(Actor);
        // If the actor indeed belongs to HouseActor, remember it.
        if (HouseActor) {
            Houses.Emplace(HouseActor, HouseActor->GetActorLocation(), HouseActor->GetName());
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Registered %d houses"), Houses.Num());
    Houses.Sort([] (const House& lhs, const House& rhs) {
        return lhs.Name < rhs.Name;
    });

    for (int Index = 0; Index < Houses.Num(); ++Index) {
        auto* Actor = Houses[Index].Actor;
        Actor->SetHouseIndex(Index);
        UE_LOG(LogTemp, Warning, TEXT("House %d: %s"), Index, *Actor->GetName());
    }
}

ELab_1PlayState ALab_1GameMode::GetCurrentState() const
{
    return CurrentState;
}

void ALab_1GameMode::SetCurrentState(ELab_1PlayState NewState)
{
    CurrentState = NewState;
    // Invoke the actions associated with transitioning to new state.
    HandleNewState(CurrentState);
}

void ALab_1GameMode::HandleNewState(ELab_1PlayState NewState)
{
    switch (NewState) {
        case ELab_1PlayState::EPlaying:
        {
            break;
        }
        case ELab_1PlayState::EGameOver:
        {
            // Take control from the player and put camera into cinematic mode.
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
            PlayerController->SetCinematicMode(true, true, true);
            break;
        }
        case ELab_1PlayState::EUnknown:
        default:
            break;
    }
}

void ALab_1GameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentState == ELab_1PlayState::EGameOver) {
        return;
    }

    for (auto& House : Houses) {
        if (House.Actor->TimeoutReached()) {
            SetCurrentState(ELab_1PlayState::EGameOver);
            return;
        }
    }

    SpawnPizzaTimer += DeltaSeconds;
    if (SpawnPizzaTimer > SpawnDelay) {
        SpawnPizza();
        SpawnPizzaTimer -= SpawnDelay;
    }
}

int ALab_1GameMode::GetPendingPizzaOrderCount() const
{
    return TotalPizzaOrderCount - DeliveredPizzaOrderCount;
}

int ALab_1GameMode::GetDeliveredPizzaOrderCount() const
{
    return DeliveredPizzaOrderCount;
}

float ALab_1GameMode::GetDeliveredPizzaAverageWaitTime() const
{
    if (DeliveredPizzaOrderCount == 0) {
        return 0.0;
    }
    return DeliveredPizzaWaitTime / DeliveredPizzaOrderCount;
}

float ALab_1GameMode::GetDeliveredPizzaPercentileWaitTime(float percentile) const
{
    int OrderCount = DeliveredPizzaWaitTimes.Num();
    if (OrderCount == 0) {
        return 0.0;
    }
    int K = FMath::Min(OrderCount, 1000);
    std::vector<float> times(K);
    for (int i = OrderCount - 1; i >= OrderCount - K; --i) {
        times[i - OrderCount + K] = DeliveredPizzaWaitTimes[i];
    }
    int PercentilePos = percentile * K;
    std::nth_element(times.begin(), times.begin() + PercentilePos, times.end());
    return times[PercentilePos];
}

void ALab_1GameMode::SpawnPizza()
{
    if (Houses.Num() == 0) {
        return;
    }
    int HouseIndex = RandomStream.RandRange(0, Houses.Num() - 1);
    auto* Actor = Houses[HouseIndex].Actor;
    int OrderNumber = TotalPizzaOrderCount++;
    auto Order = Actor->OrderPizzaDelivery(OrderNumber);
    PizzaOrders.Add(Order);
    UE_LOG(LogTemp, Warning, TEXT("Spawning pizza at %d, order number %d"), HouseIndex, OrderNumber);
}

TArray<FPizzaOrder> ALab_1GameMode::GetPizzaOrders() const
{
    TArray<FPizzaOrder> Orders;
    for (auto Order : PizzaOrders) {
        Orders.Add(Order.Get());
    }
    return Orders;
}

TArray<FVector> ALab_1GameMode::GetHouseLocations() const
{
    TArray<FVector> Locations;
    for (auto House : Houses) {
        Locations.Add(House.Location);
    }
    return Locations;
}

TSharedPtr<FPizzaOrder> ALab_1GameMode::FindOrder(int OrderNumber)
{
    for (auto& Order : PizzaOrders) {
        if (Order->OrderNumber == OrderNumber) {
            return Order;
        }
    }
    return nullptr;
}

void ALab_1GameMode::RemoveOrder(int OrderNumber)
{
    int Index = 0;
    for (; Index < PizzaOrders.Num(); ++Index) {
        if (PizzaOrders[Index]->OrderNumber == OrderNumber) {
            break;
        }
    }
    if (Index != PizzaOrders.Num()) {
        DeliveredPizzaWaitTime += PizzaOrders[Index]->CurrentWaitTime;
        DeliveredPizzaWaitTimes.Add(PizzaOrders[Index]->CurrentWaitTime);
        PizzaOrders.RemoveAtSwap(Index);
    } else {
        UE_LOG(LogTemp, Warning, TEXT("Failed to remove non-exising order %d"), OrderNumber);
    }
}

bool ALab_1GameMode::TryDeliverPizza(ALab_1Character* Character, int OrderNumber)
{
    auto Order = FindOrder(OrderNumber);
    if (!Order.IsValid()) {
        UE_LOG(LogTemp, Error, TEXT("Order %d does not exist"), OrderNumber);
        return false;
    }
    int HouseNumber = Order->HouseNumber;
    auto* Actor = Houses[HouseNumber].Actor;
    bool bDelivered = Character->TryDeliverPizza(Actor, OrderNumber);
    if (bDelivered) {
        RemoveOrder(OrderNumber);
        ++DeliveredPizzaOrderCount;
    }
    return bDelivered;
}

float ALab_1GameMode::GetHouseTimeLeft(int HouseNumber)
{
    return Houses[HouseNumber].Actor->GetTimeLeft();
}

FVector ALab_1GameMode::GetWorldOrigin() const
{
    return WorldOrigin;
}

void ALab_1GameMode::SetWorldOrigin(const FVector& worldOrigin)
{
    WorldOrigin = worldOrigin;
}

FVector ALab_1GameMode::GetWorldSize() const
{
    return WorldSize;
}

void ALab_1GameMode::SetWorldSize(const FVector& worldSize)
{
    WorldSize = worldSize;
}

int ALab_1GameMode::RegisterController(ABaseAIController* Controller)
{
    int ControllerId = Controllers.Num();
    Controllers.Add(Controller);
    return ControllerId;
}

int ALab_1GameMode::GetControllerCount() const
{
    return Controllers.Num();
}

ABaseAIController* ALab_1GameMode::GetControllerById(int ControllerId)
{
    if (ControllerId >= Controllers.Num()) {
        UE_LOG(LogTemp, Error, TEXT("ControllerId is greater or equal to the number of controllers: %d >= %d"), ControllerId, Controllers.Num());
        return nullptr;
    }
    return Controllers[ControllerId];
}
