// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAIController.h"
#include "CleverAIController.generated.h"

/**
 * 
 */
UCLASS()
class LAB_1_API ACleverAIController : public ABaseAIController
{
    GENERATED_BODY()

public:
    ACleverAIController();

protected:
    virtual void Tick(float DeltaSeconds) override;

    float ComputeHouseScore(float Distance, float TimeLeft) const;

    bool bDeliveringOrder;
    int CurrentOrderNumber;
    FVector CurrentDestination;
};
