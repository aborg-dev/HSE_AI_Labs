// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "CleverAIController.h"

ACleverAIController::ACleverAIController()
{
}

void ACleverAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MovementDecisionTimer += DeltaSeconds;

    if (TryEscape()) {
        return;
    }

    if (!bHasDirection || MovementDecisionTimer > MovementDecisionPeriod) {
        if (!bHasDirection) {
            Direction = ChooseDirection();
            bHasDirection = true;
        }
        MoveInDirection(Direction);

        if (MovementDecisionTimer > MovementDecisionPeriod) {
            MovementDecisionTimer -= MovementDecisionPeriod;
        }
    }
}
