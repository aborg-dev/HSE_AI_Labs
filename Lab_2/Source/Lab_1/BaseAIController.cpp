// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "Lab_1Character.h"
#include "Kismet/GameplayStatics.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "BaseAIController.h"
#include "Lab_1GameMode.h"


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

void ABaseAIController::SetNewMoveDestination(const FVector DestLocation)
{
    auto* MyGameMode = GetGameMode();
    if (MyGameMode->GetCurrentState() == ELab_1PlayState::EGameOver) {
        return;
    }

	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}

float ABaseAIController::GetDistanceBetween(const FVector SrcLocation, const FVector DestLocation)
{
    return FVector::Dist(SrcLocation, DestLocation);
}

float ABaseAIController::GetDistanceToDestination(const FVector DestLocation)
{
	APawn* const Pawn = GetPawn();
	if (Pawn) {
        return GetDistanceBetween(Pawn->GetActorLocation(), DestLocation);
	}
    return 0.0f;
}

bool ABaseAIController::TryGrabPizza()
{
    auto* Character = GetCharacter();
    if (!Character) {
        return false;
    }
    return Character->TryGrabPizza();
}

bool ABaseAIController::TryDeliverPizza(int OrderNumber)
{
    auto* MyGameMode = GetGameMode();
    if (MyGameMode->GetCurrentState() == ELab_1PlayState::EGameOver) {
        return false;
    }

    auto* Character = GetCharacter();
    if (!Character) {
        UE_LOG(LogTemp, Warning, TEXT("Failed delivering %d, character is dead"), OrderNumber);
        return false;
    }
    return MyGameMode->TryDeliverPizza(Character, OrderNumber);
}

TArray<FVector> ABaseAIController::GetHouseLocations()
{
    auto* MyGameMode = GetGameMode();
    return MyGameMode->GetHouseLocations();
}

TArray<FPizzaOrder> ABaseAIController::GetPizzaOrders()
{
    auto* MyGameMode = GetGameMode();
    return MyGameMode->GetPizzaOrders();
}

int ABaseAIController::GetPizzaAmount()
{
    auto* Character = GetCharacter();
    if (!Character) {
        return 0;
    }
    return Character->GetPizzaAmount();
}

int ABaseAIController::GetPizzaCapacity()
{
    auto* Character = GetCharacter();
    if (!Character) {
        return 0;
    }
    return Character->GetPizzaCapacity();
}

ALab_1GameMode* ABaseAIController::GetGameMode()
{
    return Cast<ALab_1GameMode>(UGameplayStatics::GetGameMode(this));
}

ALab_1Character* ABaseAIController::GetCharacter()
{
	APawn* const Pawn = GetPawn();
    if (!Pawn) {
        return nullptr;
    }
    ALab_1Character* Character = Cast<ALab_1Character>(Pawn);
    if (!Character) {
        return nullptr;
    }
    return Character;
}

float ABaseAIController::GetHouseTimeLeft(int HouseNumber)
{
    auto* MyGameMode = GetGameMode();
    return MyGameMode->GetHouseTimeLeft(HouseNumber);
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
