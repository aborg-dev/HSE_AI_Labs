// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "DetourCrowdAIController.h"
#include "BaseAIController.generated.h"

class ALab_3Character;
class ALab_3GameMode;

/**
 * 
 */
UCLASS()
class LAB_3_API ABaseAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()
	
public:
    ABaseAIController();

    void BeginPlay() override;

    // Returns true if character can see destination
    bool CheckVisibility(FVector DestLocation);

    // Navigate player to the given world location
    void SetNewMoveDestination(FVector DestLocation);

    float GetCharacterMaxSpeed();

    int GetControllerId() const;

    FVector GetCharacterLocation();

    // Returns the coordinates of the edge of the map
    FVector GetWorldOrigin();

    // Returns size of the map
    FVector GetWorldSize();

    int GetControllerCount();

    ABaseAIController* GetControllerById(int controllerId);

    TArray<FVector> GetExitLocations();

    bool Escape(int ExitIndex);

protected:
    virtual void Tick(float DeltaSeconds) override;

    int ControllerId;

private:
    ALab_3GameMode* GetGameMode();

    ALab_3Character* GetCharacter();

    bool bEscaped;

};
