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
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/GameContent/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

    // Set the default HUD class to be used in game.
    HUDClass = ALesson_1HUD::StaticClass();

    // Initialize default value of decay rate.
    DecayRate = 0.5f;
}

void ALesson_1GameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Get our game character.
    ALesson_1Character* MyCharacter = Cast<ALesson_1Character>(UGameplayStatics::GetPlayerPawn(this, 0));

    // If the power of the character is large enough, perform a decay. Otherwise transition the game to the final state.
    if (MyCharacter->PowerLevel > 0.05) {
        // For decay formula use linear interpolation method with specified starting and finishing value, time and decay rate.
        MyCharacter->PowerLevel = FMath::FInterpTo(MyCharacter->PowerLevel, 0, DeltaSeconds, DecayRate);
    } else {
        SetCurrentState(ELesson_1PlayState::EGameOver);
    }
}

void ALesson_1GameMode::BeginPlay()
{
    // Don't forget to call parent BeginPlay() method.
    Super::BeginPlay();

    // Find all SpawnVolume actors.
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);
    for (auto Actor : FoundActors) {
        ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
        // If the actor indeed belongs to SpawnVolume, remember it.
        if (SpawnVolumeActor) {
            SpawnVolumeActors.Add(SpawnVolumeActor);
        }
    }
    // Transition the game into playing state.
    SetCurrentState(ELesson_1PlayState::EPlaying);
}

ELesson_1PlayState ALesson_1GameMode::GetCurrentState() const
{
    return CurrentState;
}

void ALesson_1GameMode::SetCurrentState(ELesson_1PlayState NewState)
{
    CurrentState = NewState;
    // Invoke the actions associated with transitioning to new state.
    HandleNewState(CurrentState);
}

void ALesson_1GameMode::HandleNewState(ELesson_1PlayState NewState)
{
    switch (NewState) {
        case ELesson_1PlayState::EPlaying:
        {
            // Turn on all spawn volumes to start creating new batteries.
            for (ASpawnVolume* Volume : SpawnVolumeActors) {
                Volume->EnableSpawning();
            }
            break;
        }
        case ELesson_1PlayState::EGameOver:
        {
            // When the game is finished turn off all spawn volumes.
            for (ASpawnVolume* Volume : SpawnVolumeActors) {
                Volume->DisableSpawning();
            }
            // Take control from the player and put camera into cinematic mode.
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
            PlayerController->SetCinematicMode(true, true, true);
            break;
        }
        case ELesson_1PlayState::EUnknown:
        default:
            break;
    }
}
