// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAIController.h"
#include "SimpleAIController.generated.h"

/**
 *
 */
UCLASS()
class LAB_3_API ASimpleAIController : public ABaseAIController
{
    GENERATED_BODY()

public:
    ASimpleAIController();

    void BeginPlay() override;

    void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = Movement)
    float InitialScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float ScaleDecayRate;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MinAllowedScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MovementDecisionPeriod;

    UPROPERTY(EditAnywhere, Category = Movement)
    float ChooseDirectionProbeCount;

private:
    FVector Direction;
    bool bHasDirection;

    FVector PreviousLocation;

    float MovementDecisionTimer;

    FRandomStream RandomStream;

    void MoveInDirection(FVector direction);

    float GetDirectionScale(FVector direction);

    FVector GetAngleDirection(float angle);

    FVector GetRandomDirection();

    FVector ChooseDirection();

    bool TryEscape();
};
