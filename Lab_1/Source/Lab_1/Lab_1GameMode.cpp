// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_1.h"
#include "Lab_1GameMode.h"
#include "Lab_1PlayerController.h"
#include "Lab_1Character.h"
#include "PizzaHUD.h"
#include "HouseActor.h"

ALab_1GameMode::ALab_1GameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ALab_1PlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/GameContent/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

    // Set the default HUD class to be used in game.
    HUDClass = APizzaHUD::StaticClass();

    // Spawn pizza every 3 seconds.
    SpawnDelay = 2.0f;

    TotalPizzaOrderCount = 0;
    DeliveredPizzaOrderCount = 0;
}

void ALab_1GameMode::BeginPlay()
{
    // Don't forget to call parent BeginPlay() method.
    Super::BeginPlay();

    // Find all SpawnVolume actors.
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHouseActor::StaticClass(), FoundActors);
    for (auto Actor : FoundActors) {
        AHouseActor* HouseActor = Cast<AHouseActor>(Actor);
        // If the actor indeed belongs to SpawnVolume, remember it.
        if (HouseActor) {
            HouseActors.Add(HouseActor);
            HouseLocations.Add(HouseActor->GetActorLocation());
        }
    }

    SpawnPizzaTimer = 0.0f;
    RandomStream.Initialize(42);
    // Transition the game into playing state.
    SetCurrentState(ELab_1PlayState::EPlaying);
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

    for (auto House : HouseActors) {
        if (House->TimeoutReached()) {
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

void ALab_1GameMode::SpawnPizza()
{
    TArray<AHouseActor*> HouseActorsNotWaitingDelivery;
    TArray<int> RealIndices;
    for (int Index = 0; Index < HouseActors.Num(); ++Index) {
        auto Actor = HouseActors[Index];
        if (!Actor->WaitsPizzaDelivery()) {
            HouseActorsNotWaitingDelivery.Add(Actor);
            RealIndices.Add(Index);
        }
    }
    if (HouseActorsNotWaitingDelivery.Num() == 0) {
        // Game over.
        return;
    }

    int HouseIndex = RandomStream.RandRange(0, HouseActorsNotWaitingDelivery.Num() - 1);
    auto Actor = HouseActorsNotWaitingDelivery[HouseIndex];
    int RealHouseIndex = RealIndices[HouseIndex];
    int OrderNumber = TotalPizzaOrderCount++;
    PizzaOrders.Add(FPizzaOrder(OrderNumber, RealHouseIndex, 1));
    UE_LOG(LogTemp, Warning, TEXT("Spawning pizza at %d"), RealHouseIndex);
    Actor->OrderPizzaDelivery();
}

TArray<FPizzaOrder> ALab_1GameMode::GetPizzaOrders() const
{
    return PizzaOrders;
}

const TArray<FVector>& ALab_1GameMode::GetHouseLocations() const
{
    return HouseLocations;
}

FPizzaOrder* ALab_1GameMode::FindOrder(int OrderNumber)
{
    for (auto& Order : PizzaOrders) {
        if (Order.OrderNumber == OrderNumber) {
            return &Order;
        }
    }
    return nullptr;
}

void ALab_1GameMode::RemoveOrder(int OrderNumber)
{
    int Index = 0;
    for (; Index < PizzaOrders.Num(); ++Index) {
        if (PizzaOrders[Index].OrderNumber == OrderNumber) {
            break;
        }
    }
    if (Index != PizzaOrders.Num()) {
        PizzaOrders.RemoveAt(Index, 1, true);
    } else {
        UE_LOG(LogTemp, Warning, TEXT("Failed to remove non-exising order %d"), OrderNumber);
    }
}

bool ALab_1GameMode::TryDeliverPizza(ALab_1Character* Character, int OrderNumber)
{
    auto* Order = FindOrder(OrderNumber);
    if (!Order) {
        UE_LOG(LogTemp, Error, TEXT("Order %d does not exist"), OrderNumber);
        return false;
    }
    int HouseNumber = Order->HouseNumber;
    auto* House = HouseActors[HouseNumber];
    bool bDelivered = Character->TryDeliverPizza(House);
    if (bDelivered) {
        RemoveOrder(OrderNumber);
        ++DeliveredPizzaOrderCount;
    }
    return bDelivered;
}
