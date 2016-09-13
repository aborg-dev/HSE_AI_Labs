// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lesson_1.h"
#include "Lesson_1GameMode.h"
#include "Lesson_1Character.h"
#include "SpawnVolume.h"
#include "Lesson_1HUD.h"

#include "Kismet/GameplayStatics.h"

ALesson_1GameMode::ALesson_1GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

    HUDClass = ALesson_1HUD::StaticClass();

    DecayRate = 0.5f;
}

void ALesson_1GameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ALesson_1Character* MyCharacter = Cast<ALesson_1Character>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (MyCharacter->PowerLevel > 0.05) {
        MyCharacter->PowerLevel = FMath::FInterpTo(MyCharacter->PowerLevel, 0, DeltaSeconds, DecayRate);
    } else {
        SetCurrentState(ELesson_1PlayState::EGameOver);
    }
}

ELesson_1PlayState ALesson_1GameMode::GetCurrentState() const
{
    return CurrentState;
}

void ALesson_1GameMode::SetCurrentState(ELesson_1PlayState NewState)
{
    CurrentState = NewState;
    HandleNewState(CurrentState);
}

void ALesson_1GameMode::HandleNewState(ELesson_1PlayState NewState)
{
    switch (NewState) {
        case ELesson_1PlayState::EPlaying:
        {
            for (ASpawnVolume* Volume : SpawnVolumeActors) {
                Volume->EnableSpawning();
            }
            break;
        }
        case ELesson_1PlayState::EGameOver:
        {
            for (ASpawnVolume* Volume : SpawnVolumeActors) {
                Volume->DisableSpawning();
            }
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
            PlayerController->SetCinematicMode(true, true, true);
            break;
        }
        case ELesson_1PlayState::EUnknown:
        default:
            break;
    }
}

void ALesson_1GameMode::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);
    for (auto Actor : FoundActors) {
        ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
        if (SpawnVolumeActor) {
            SpawnVolumeActors.Add(SpawnVolumeActor);
        }
    }
    SetCurrentState(ELesson_1PlayState::EPlaying);
}