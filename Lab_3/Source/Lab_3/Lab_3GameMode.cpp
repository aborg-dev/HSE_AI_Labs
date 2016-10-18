// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_3.h"
#include "Lab_3GameMode.h"
#include "Lab_3PlayerController.h"
#include "Lab_3Character.h"
#include "MazeHUD.h"
#include "MazeExit.h"
#include "BaseAIController.h"

ALab_3GameMode::ALab_3GameMode()
{
    // Set the default HUD class to be used in game.
    HUDClass = AMazeHUD::StaticClass();

    TimePassed = 0.0f;
}

void ALab_3GameMode::BeginPlay()
{
    // Don't forget to call parent BeginPlay() method.
    Super::BeginPlay();

    DiscoverExits();

    RandomStream.Initialize(42);
    // Transition the game into playing state.
    SetCurrentState(ELab_3PlayState::EPlaying);

    EscapedControllerCount = 0;
}

void ALab_3GameMode::DiscoverExits()
{
    // Find all exit actors.
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMazeExit::StaticClass(), FoundActors);
    for (auto Actor : FoundActors) {
        AMazeExit* ExitActor = Cast<AMazeExit>(Actor);
        // If the actor indeed belongs to MazeExit, remember it.
        if (ExitActor) {
            Exits.Emplace(ExitActor, ExitActor->GetActorLocation(), ExitActor->GetName());
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Registered %d exits"), Exits.Num());
    Exits.Sort([] (const Exit& lhs, const Exit& rhs) {
        return lhs.Name < rhs.Name;
    });

    for (int Index = 0; Index < Exits.Num(); ++Index) {
        auto* Actor = Exits[Index].Actor;
        Actor->SetExitIndex(Index);
        UE_LOG(LogTemp, Warning, TEXT("Exit %d: %s"), Index, *Actor->GetName());
    }
}

ELab_3PlayState ALab_3GameMode::GetCurrentState() const {
    return CurrentState;
}

void ALab_3GameMode::SetCurrentState(ELab_3PlayState NewState)
{
    CurrentState = NewState;
    // Invoke the actions associated with transitioning to new state.
    HandleNewState(CurrentState);
}

void ALab_3GameMode::HandleNewState(ELab_3PlayState NewState)
{
    switch (NewState) {
        case ELab_3PlayState::EPlaying:
        {
            break;
        }
        case ELab_3PlayState::EGameOver:
        {
            // Take control from the player and put camera into cinematic mode.
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
            PlayerController->SetCinematicMode(true, true, true);
            break;
        }
        case ELab_3PlayState::EUnknown:
        default:
            break;
    }
}

void ALab_3GameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentState == ELab_3PlayState::EGameOver) {
        return;
    }
    TimePassed += DeltaSeconds;
}

FVector ALab_3GameMode::GetWorldOrigin() const
{
    return WorldOrigin;
}

void ALab_3GameMode::SetWorldOrigin(const FVector& worldOrigin)
{
    WorldOrigin = worldOrigin;
}

FVector ALab_3GameMode::GetWorldSize() const
{
    return WorldSize;
}

void ALab_3GameMode::SetWorldSize(const FVector& worldSize)
{
    WorldSize = worldSize;
}

int ALab_3GameMode::RegisterController(ABaseAIController* Controller)
{
    int ControllerId = Controllers.Num();
    Controllers.Add(Controller);
    bControllerEscaped.Add(false);
    return ControllerId;
}

int ALab_3GameMode::GetControllerCount() const
{
    return Controllers.Num();
}

ABaseAIController* ALab_3GameMode::GetControllerById(int ControllerId)
{
    if (ControllerId >= Controllers.Num()) {
        UE_LOG(LogTemp, Error, TEXT("ControllerId is greater or equal to the number of controllers: %d >= %d"), ControllerId, Controllers.Num());
        return nullptr;
    }
    return Controllers[ControllerId];
}

float ALab_3GameMode::GetTimePassed() const
{
    return TimePassed;
}

TArray<FVector> ALab_3GameMode::GetExitLocations()
{
    TArray<FVector> exitLocations;
    for (const auto& exit : Exits) {
        exitLocations.Add(exit.Location);
    }
    return exitLocations;
}

bool ALab_3GameMode::Escape(int exitIndex, int controllerId)
{
    if (controllerId >= Controllers.Num()) {
        UE_LOG(LogTemp, Error, TEXT("Controller id is greater or equal to the number of controllers: %d >= %d"), controllerId, Controllers.Num());
        return false;
    }

    if (bControllerEscaped[controllerId]) {
        UE_LOG(LogTemp, Error, TEXT("Controller %d has already escaped"), controllerId);
        return false;
    }

    if (exitIndex >= Exits.Num()) {
        UE_LOG(LogTemp, Error, TEXT("Exit index is greater or equal to the number of exits: %d >= %d"), exitIndex, Exits.Num());
        return false;
    }

    auto characterLocation = Controllers[controllerId]->GetCharacterLocation();
    auto bEscaped = Exits[exitIndex].Actor->TryEscape(characterLocation);
    if (bEscaped) {
        UE_LOG(LogTemp, Error, TEXT("Controller %d has escaped"), controllerId);
        ++EscapedControllerCount;
        bControllerEscaped[controllerId] = true;

        if (EscapedControllerCount == Controllers.Num()) {
            SetCurrentState(ELab_3PlayState::EGameOver);
        }
    }
    return bEscaped;
}
