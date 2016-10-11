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

    void BeginPlay() override;

    /** Navigate player to the given world location. */
    void SetNewMoveDestination(const FVector DestLocation);

    float GetDistanceToDestination(const FVector DestLocation);

    float GetDistanceBetween(const FVector SrcLocation, const FVector DestLocation);

    bool TryDeliverPizza(int OrderNumber);

    bool TryGrabPizza();

    int GetPizzaAmount();

    int GetPizzaCapacity();

    float GetCharacterMaxSpeed();

    int GetControllerId() const;

    FVector GetCharacterLocation();

    // Returns the coordinates of the edge of the map
    FVector GetWorldOrigin();

    // Returns size of the map
    FVector GetWorldSize();

    int GetControllerCount();

    ABaseAIController* GetControllerById(int controllerId);

protected:
    virtual void Tick(float DeltaSeconds) override;

    // Returns locations of all houses in the level.
    TArray<FVector> GetHouseLocations();

    // Returns currently active pizza orders.
    TArray<FPizzaOrder> GetPizzaOrders();

    float GetHouseTimeLeft(int HouseNumber);

    int ControllerId;

private:
    ALab_1GameMode* GetGameMode();

    ALab_1Character* GetCharacter();

};
