// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "CleverAIController.h"

ACleverAIController::ACleverAIController()
{
    ChooseDirectionProbeCount = 4;
    bIsMoving = false;
}

void ACleverAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsMoving) {
        // Check arrival
    } else {
        DiscoverNeighborhood();
    }
}

void ACleverAIController::DiscoverNeighborhood()
{
    auto currentLocation = GetCharacterLocation();
    for (float angle = 0.0f; angle <= 2 * PI; angle += 2 * PI / ChooseDirectionProbeCount) {
        auto direction = GetAngleDirection(angle);
        auto scale = GetDirectionScale(direction);
        if (scale < MinAllowedScale) {
            continue;
        }
        auto nextLocation
    }
}
