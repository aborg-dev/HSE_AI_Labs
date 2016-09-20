// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "PizzaOrder.h"
#include "BaseAIController.generated.h"

class ALab_1Character;
class ALab_1GameMode;

/**
 * 
 */
UCLASS()
class LAB_1_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    ABaseAIController();

protected:
    virtual void Tick(float DeltaSeconds) override;

    /** Navigate player to the given world location. */
    void SetNewMoveDestination(const FVector DestLocation);

    float GetDistanceToDestination(const FVector DestLocation);

    bool TryDeliverPizza(int OrderNumber);

    bool TryGrabPizza();

    // Returns locations of all houses in the level.
    const TArray<FVector>& GetHouseLocations();

    // Returns currently active pizza orders.
    TArray<FPizzaOrder> GetPizzaOrders();

    int GetPizzaAmount();

    int GetPizzaCapacity();

    float GetHouseTimeLeft(int HouseNumber);

private:
    ALab_1GameMode* GetGameMode();

    ALab_1Character* GetCharacter();

};
