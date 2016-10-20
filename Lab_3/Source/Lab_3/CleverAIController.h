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

    UPROPERTY(EditAnywhere, Category = Movement)
    int ChooseDirectionProbeCount;

    UPROPERTY(EditAnywhere, Category = Movement)
    float MinAllowedScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float InitialScale;

    UPROPERTY(EditAnywhere, Category = Movement)
    float ScaleDecayRate;

private:
    NavGraph Graph;

    int CurrentVertex;

    int NextVertex;
    FVector NextVertexLocation;

    TArray<int> TraversalStack;
    TSet<int> DiscoveredVertices;

    bool bIsMoving;

    void DiscoverNeighborhood();

    bool ChooseDirection();

    void GoToVertex(int vertexIndex);

    float GetDirectionScale(FVector direction);

};
