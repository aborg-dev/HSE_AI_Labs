// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseAIController.h"
#include "NavGraph.h"
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

    int GetCurrentVertex();

    TSharedPtr<NavGraph> GetNavigationGraph();

    UPROPERTY(EditAnywhere, Category = Movement)
    int ChooseDirectionProbeCount;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MinAllowedScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float InitialScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float ScaleDecayRate;

    UPROPERTY(EditAnywhere, Category = Movement)
    float AcceptableDistanceToTarget;

    UPROPERTY(EditAnywhere, Category = Movement)
    float OvershootDistance;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MinTryMoveTime;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MinAllowedSpeed;

private:
    TSharedPtr<NavGraph> Graph;

    int CurrentVertex;

    int NextVertex;
    FVector NextVertexLocation;
    float PreviousDistanceToNextVertex;
    float TryMoveTime;

    TArray<int> TraversalStack;

    bool bIsMoving;
    bool bIsMovingBack;

    void DiscoverNeighborhood();

    bool ChooseDirection();

    void GoToVertex(FVector vertexLocation);

    float GetDirectionScale(FVector direction);

    bool TryEscape();

};
