// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "Kismet/GameplayStatics.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "BaseAIController.h"
#include "Lab_3GameMode.h"
#include "Lab_3Character.h"
#include "StaticLibrary.h"


ABaseAIController::ABaseAIController()
{
    ControllerId = -1;
}

void ABaseAIController::BeginPlay()
{
    Super::BeginPlay();

    auto* MyGameMode = GetGameMode();
    if (!MyGameMode) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to register controller in GameMode"));
    }
    ControllerId = MyGameMode->RegisterController(this);
}

void ABaseAIController::Tick(float DeltaSeconds)
{
}

bool ABaseAIController::CheckVisibility(FVector DestLocation)
{
    APawn* const Pawn = GetPawn();
    if (Pawn) {
        FHitResult HitData(ForceInit);
        DestLocation.Z += 50.0f;
        return !UStaticLibrary::Trace(GetWorld(), Pawn, Pawn->GetActorLocation(), DestLocation, HitData);
    }
    return false;
}

void ABaseAIController::SetNewMoveDestination(FVector DestLocation)
{
    auto* MyGameMode = GetGameMode();
    if (MyGameMode->GetCurrentState() == ELab_3PlayState::EGameOver) {
        return;
    }

    APawn* const Pawn = GetPawn();
    if (Pawn) {
        FHitResult HitData(ForceInit);
        DestLocation.Z += 50.0f;
        if (UStaticLibrary::Trace(GetWorld(), Pawn, Pawn->GetActorLocation(), DestLocation, HitData)) {
            return;
        }

        UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
        float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

        // We need to issue move command only if far enough in order for walk animation to play correctly
        if (NavSys && (Distance > 120.0f))
        {
            NavSys->SimpleMoveToLocation(this, DestLocation);
        }
    }
}

ALab_3GameMode* ABaseAIController::GetGameMode()
{
    return Cast<ALab_3GameMode>(UGameplayStatics::GetGameMode(this));
}

ALab_3Character* ABaseAIController::GetCharacter()
{
	APawn* const Pawn = GetPawn();
    if (!Pawn) {
        return nullptr;
    }
    ALab_3Character* Character = Cast<ALab_3Character>(Pawn);
    if (!Character) {
        return nullptr;
    }
    return Character;
}

float ABaseAIController::GetCharacterMaxSpeed()
{
    auto* Character = GetCharacter();
    if (!Character) {
        return 0.f;
    }
    return Character->GetCharacterMovement()->MaxWalkSpeed;
}

FVector ABaseAIController::GetCharacterLocation()
{
	APawn* const Pawn = GetPawn();
	if (!Pawn) {
        return FVector(0.f, 0.f, 0.f);
	}
    return Pawn->GetActorLocation();
}

FVector ABaseAIController::GetWorldOrigin()
{
    auto* MyGameMode = GetGameMode();
    if (!MyGameMode) {
        return FVector(0.f, 0.f, 0.f);
    }
    return MyGameMode->GetWorldOrigin();
}

FVector ABaseAIController::GetWorldSize()
{
    auto* MyGameMode = GetGameMode();
    if (!MyGameMode) {
        return FVector(0.f, 0.f, 0.f);
    }
    return MyGameMode->GetWorldSize();
}

int ABaseAIController::GetControllerId() const
{
    if (ControllerId == -1) {
        UE_LOG(LogTemp, Warning, TEXT("ControllerId was not set"));
    }
    return ControllerId;
}

int ABaseAIController::GetControllerCount()
{
    auto* MyGameMode = GetGameMode();
    if (!MyGameMode) {
        return 0;
    }
    return MyGameMode->GetControllerCount();
}

ABaseAIController* ABaseAIController::GetControllerById(int controllerId)
{
    auto* MyGameMode = GetGameMode();
    if (!MyGameMode) {
        return nullptr;
    }
    return MyGameMode->GetControllerById(controllerId);
}
