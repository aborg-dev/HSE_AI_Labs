// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "PizzaOrder.h"
#include "Lab_1GameMode.generated.h"

class AHouseActor;
class ALab_1Character;
class ABaseAIController;

// Enum to represent current game state.
enum class ELab_1PlayState : short
{
    EPlaying,  // set when the game is active
    EGameOver, // set when the game is over
    EUnknown   // default value
};

UCLASS(minimalapi)
class ALab_1GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALab_1GameMode();

    virtual void Tick(float DeltaSeconds) override;

    // Initializes game logic when game starts.
    virtual void BeginPlay() override;

    // Spawning delay.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawning)
    float SpawnDelay;

    // Returns locations of all houses in the level.
    TArray<FVector> GetHouseLocations() const;

    // Returns currently active pizza orders.
    TArray<FPizzaOrder> GetPizzaOrders() const;

    bool TryDeliverPizza(ALab_1Character* Character, int OrderNumber);

    int GetPendingPizzaOrderCount() const;

    int GetDeliveredPizzaOrderCount() const;

    float GetDeliveredPizzaAverageWaitTime() const;

    float GetDeliveredPizzaPercentileWaitTime(float percentile) const;

    // Returns current game state.
    ELab_1PlayState GetCurrentState() const;

    // Sets game state to new value.
    void SetCurrentState(ELab_1PlayState NewState);

    float GetHouseTimeLeft(int HouseNumber);

    void DiscoverHouses();

    FVector GetWorldOrigin() const;

    void SetWorldOrigin(const FVector& worldOrigin);

    FVector GetWorldSize() const;

    void SetWorldSize(const FVector& worldSize);

    int RegisterController(ABaseAIController* Controller);

    int GetControllerCount() const;

    ABaseAIController* GetControllerById(int ControllerId);

private:
    struct House
    {
        House(AHouseActor* actor, const FVector& location, const FString& name)
            : Actor(actor), Location(location), Name(name) {}

        AHouseActor* Actor;
        FVector Location;
        FString Name;
    };

    FVector WorldOrigin;
    FVector WorldSize;

    TArray<House> Houses;
    TArray<TSharedRef<FPizzaOrder>> PizzaOrders;
    TArray<ABaseAIController*> Controllers;

    TSharedPtr<FPizzaOrder> FindOrder(int OrderNumber);

    void RemoveOrder(int OrderNumber);

    float SpawnPizzaTimer;

    int TotalPizzaOrderCount;

    int DeliveredPizzaOrderCount;

    float DeliveredPizzaWaitTime;

    TArray<float> DeliveredPizzaWaitTimes;

    void SpawnPizza();

    FRandomStream RandomStream;

    // Stores current game state.
    ELab_1PlayState CurrentState;

    // Handles game state changes.
    void HandleNewState(ELab_1PlayState NewState);

};



