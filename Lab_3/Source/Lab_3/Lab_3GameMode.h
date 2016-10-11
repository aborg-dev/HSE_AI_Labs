// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "Lab_3GameMode.generated.h"

// Enum to represent current game state.
enum class ELab_3PlayState : short
{
    EPlaying,  // set when the game is active
    EGameOver, // set when the game is over
    EUnknown   // default value
};

class ABaseAIController;

UCLASS(minimalapi)
class ALab_3GameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ALab_3GameMode();

    virtual void Tick(float DeltaSeconds) override;

    // Initializes game logic when game starts.
    virtual void BeginPlay() override;

    // Returns current game state.
    ELab_3PlayState GetCurrentState() const;

    // Sets game state to new value.
    void SetCurrentState(ELab_3PlayState NewState);

    FVector GetWorldOrigin() const;

    void SetWorldOrigin(const FVector& worldOrigin);

    FVector GetWorldSize() const;

    void SetWorldSize(const FVector& worldSize);

    int RegisterController(ABaseAIController* Controller);

    int GetControllerCount() const;

    ABaseAIController* GetControllerById(int ControllerId);

    float GetTimePassed() const;

private:
    FVector WorldOrigin;
    FVector WorldSize;

    TArray<ABaseAIController*> Controllers;

    // Stores current game state.
    ELab_3PlayState CurrentState;

    FRandomStream RandomStream;

    float TimePassed;

    // Handles game state changes.
    void HandleNewState(ELab_3PlayState NewState);
};



