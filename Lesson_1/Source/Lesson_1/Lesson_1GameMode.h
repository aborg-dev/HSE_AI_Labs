// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"

#include "Lesson_1GameMode.generated.h"

enum class ELesson_1PlayState : short
{
    EPlaying,
    EGameOver,
    EUnknown
};

class ASpawnVolume;

UCLASS(minimalapi)
class ALesson_1GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALesson_1GameMode();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Power)
    float DecayRate;

    ELesson_1PlayState GetCurrentState() const;

    void SetCurrentState(ELesson_1PlayState NewState);

    virtual void BeginPlay() override;

private:
    TArray<ASpawnVolume*> SpawnVolumeActors;

    ELesson_1PlayState CurrentState;

    void HandleNewState(ELesson_1PlayState NewState);

};


