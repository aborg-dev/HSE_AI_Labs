// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_3.h"
#include "Lab_3GameMode.h"
#include "Lab_3PlayerController.h"
#include "Lab_3Character.h"
#include "MazeHUD.h"

ALab_3GameMode::ALab_3GameMode()
{
    // TODO: Get rid of player contoller
    // use our custom PlayerController class
    PlayerControllerClass = ALab_3PlayerController::StaticClass();

    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // Set the default HUD class to be used in game.
    HUDClass = AMazeHUD::StaticClass();

    TimePassed = 0.0f;
}

void ALab_3GameMode::BeginPlay()
{
    // Don't forget to call parent BeginPlay() method.
    Super::BeginPlay();

    RandomStream.Initialize(42);
    // Transition the game into playing state.
    SetCurrentState(ELab_3PlayState::EPlaying);
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
