// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "PizzaOrder.h"
#include "Lab_1GameMode.generated.h"

class AHouseActor;
class ALab_1Character;

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
    const TArray<FVector>& GetHouseLocations() const;

    // Returns currently active pizza orders.
    TArray<FPizzaOrder> GetPizzaOrders() const;

    bool TryDeliverPizza(ALab_1Character* Character, int OrderNumber);

    int GetPendingPizzaOrderCount() const;

    int GetDeliveredPizzaOrderCount() const;

private:
    TArray<AHouseActor*> HouseActors;
    TArray<FVector> HouseLocations;
    TArray<FPizzaOrder> PizzaOrders;

    FPizzaOrder* FindOrder(int OrderNumber);

    void RemoveOrder(int OrderNumber);

    float SpawnPizzaTimer;

    int TotalPizzaOrderCount;

    int DeliveredPizzaOrderCount;

    void SpawnPizza();

    FRandomStream RandomStream;

};



