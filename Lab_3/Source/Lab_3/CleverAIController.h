// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAIController.h"
#include "CleverAIController.generated.h"

/**
 *
 */
UCLASS()
class LAB_3_API ACleverAIController : public ABaseAIController
{
    GENERATED_BODY()

public:
    ACleverAIController();

    void BeginPlay() override;

    void Tick(float DeltaSeconds) override;


};
