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

    int GetCurrentOrderNumber();

protected:
    void ActAsMaster();

    void ActAsWorker();

    void TryDeliverOrder();

    void RefillPizzaIfNeeded();

    void SetPendingOrderNumber(int OrderNumber);

    virtual void Tick(float DeltaSeconds) override;

    FVector GetHouseLocation(int HouseNumber);

    float ComputeHouseScore(float Distance, float TimeLeft) const;

    float GetTimeToCoverDistance(float distance);

    bool bDeliveringOrder;
    int CurrentOrderNumber;
    int PendingOrderNumber;
    FVector CurrentDestination;

    float MasterDecisionTimer;

    static constexpr int MASTER_ID = 0;
    static constexpr float MASTER_DECISION_PERIOD = 0.2f;
};
