// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "SimpleAIController.h"

#include "MazeExit.h"

ASimpleAIController::ASimpleAIController()
{
    InitialScale = 100.0f;
    ScaleDecayRate = 0.8f;
    MinAllowedScale = 1.0f;

    bHasDirection = false;
    MovementDecisionPeriod = 1.0f;
    MovementDecisionTimer = 0.0f;
    PreviousLocation = GetCharacterLocation();

    ChooseDirectionProbeCount = 4;
}

void ASimpleAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ASimpleAIController::Tick(float DeltaSeconds)
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

bool ASimpleAIController::TryEscape()
{
    auto exitLocations = GetExitLocations();
    auto currentLocation = GetCharacterLocation();
    for (int exitIndex = 0; exitIndex < exitLocations.Num(); ++exitIndex) {
        const auto& location = exitLocations[exitIndex];
        auto distance = (currentLocation - location).Size();
        if (distance < AMazeExit::EscapeRadius) {
            bool bSuccess = Escape(exitIndex);
            if (bSuccess) {
                return true;
            }
        }
    }
    return false;
}

float ASimpleAIController::GetDirectionScale(FVector direction)
{
    auto currentLocation = GetCharacterLocation();
    float scale = InitialScale;
    while (!CheckVisibility(currentLocation + scale * direction)) {
        scale *= ScaleDecayRate;
    }
    return scale;
}

void ASimpleAIController::MoveInDirection(FVector direction)
{
    auto currentLocation = GetCharacterLocation();
    float scale = GetDirectionScale(direction);
    if (scale < MinAllowedScale) {
        bHasDirection = false;
    } else {
        PreviousLocation = currentLocation;
        SetNewMoveDestination(currentLocation + scale * direction);
    }
}

FVector ASimpleAIController::GetAngleDirection(float angle)
{
    FVector direction;
    FMath::SinCos(&direction.X, &direction.Y, angle);
    direction.Z = 0;
    return direction;
}

FVector ASimpleAIController::GetRandomDirection()
{
    float angle = FMath::DegreesToRadians(RandomStream.RandRange(0.0f, 360.0f));
    return GetAngleDirection(angle);
}

FVector ASimpleAIController::ChooseDirection()
{
    auto currentLocation = GetCharacterLocation();
    if (PreviousLocation == currentLocation) {
        return GetRandomDirection();
    }

    auto deltaPosition = PreviousLocation - currentLocation;
    deltaPosition /= deltaPosition.Size();

    bool bFoundDirection = false;
    FVector bestDirection;
    float bestAngleDiff;
    for (float angle = 0.0f; angle <= 360.0f; angle += 360.0f / ChooseDirectionProbeCount) {
        auto direction = GetAngleDirection(FMath::DegreesToRadians(angle));
        if (GetDirectionScale(direction) < MinAllowedScale) {
            continue;
        }

        float angleDiff = acosf(FVector::DotProduct(deltaPosition, direction));
        if (!bFoundDirection || angleDiff > bestAngleDiff) {
            bestAngleDiff = angleDiff;
            bestDirection = direction;
            bFoundDirection = true;
        }
    }

    if (!bFoundDirection) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to found direction at %s"), *currentLocation.ToString());
        return GetRandomDirection();
    }

    return bestDirection;
}
