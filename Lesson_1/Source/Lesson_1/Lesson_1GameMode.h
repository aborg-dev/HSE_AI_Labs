// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "Lesson_1GameMode.generated.h"

// Enum to represent current game state.
enum class ELesson_1PlayState : short
{
    EPlaying,  // set when the game is active
    EGameOver, // set when the game is over
    EUnknown   // default value
};

class ASpawnVolume;

UCLASS(minimalapi)
class ALesson_1GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALesson_1GameMode();

    // Override the method to decrease character power on every tick.
    virtual void Tick(float DeltaSeconds) override;

    // Initializes game logic when game starts.
    virtual void BeginPlay() override;

    // Defines how much the power of the character will be drained with time.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Power)
    float DecayRate;

    // Returns current game state.
    ELesson_1PlayState GetCurrentState() const;

    // Sets game state to new value.
    void SetCurrentState(ELesson_1PlayState NewState);

private:
    // Stores all SpawnVolume actors on the level to allow fast access.
    TArray<ASpawnVolume*> SpawnVolumeActors;

    // Stores current game state.
    ELesson_1PlayState CurrentState;

    // Handles game state changes.
    void HandleNewState(ELesson_1PlayState NewState);

};


