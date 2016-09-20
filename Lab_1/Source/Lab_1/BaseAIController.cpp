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

float ABaseAIController::GetDistanceToDestination(const FVector DestLocation)
{
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		return FVector::Dist(DestLocation, Pawn->GetActorLocation());
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
        return;
    }

    auto* MyGameMode = GetGameMode();
    auto* Character = GetCharacter();
    if (!Character) {
        UE_LOG(LogTemp, Warning, TEXT("Failed delivering %d, character is dead"), OrderNumber);
        return false;
    }
    return MyGameMode->TryDeliverPizza(Character, OrderNumber);
}

const TArray<FVector>& ABaseAIController::GetHouseLocations()
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

