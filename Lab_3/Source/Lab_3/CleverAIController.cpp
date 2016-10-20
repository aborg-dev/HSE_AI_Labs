// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "CleverAIController.h"
#include "StaticLibrary.h"

const int NO_VERTEX = -1;

ACleverAIController::ACleverAIController()
{
    ChooseDirectionProbeCount = 4;
    MinAllowedScale = 50.0f;
    InitialScale = 300.0f;
    ScaleDecayRate = 0.9f;
    bIsMoving = false;
    CurrentVertex = NO_VERTEX;
}

void ACleverAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentVertex == NO_VERTEX) {
        CurrentVertex = Graph.AddVertex(GetCharacterLocation());
        TraversalStack.Add(CurrentVertex);
        DiscoveredVertices.Add(CurrentVertex);
    }

    if (bIsMoving) {
        auto currentLocation = GetCharacterLocation();
        if ((currentLocation - NextVertexLocation).Size() < 1e-3) {
            bIsMoving = false;
            CurrentVertex = NextVertex;
            NextVertex = NO_VERTEX;
        } else {
            return;
        }
    }

    DiscoverNeighborhood();
    if (!ChooseDirection()) {
        UE_LOG(LogTemp, Warning, TEXT("Traversal completed"));
    }
}

void ACleverAIController::GoToVertex(int vertexIndex)
{
    NextVertex = vertexIndex;
    NextVertexLocation = Graph.GetVertexByIndex(vertexIndex);
    bIsMoving = true;
    SetNewMoveDestination(NextVertexLocation);
}

bool ACleverAIController::ChooseDirection()
{
    if (TraversalStack.Num() == 0) {
        return false;
    }

    int currentVertex = TraversalStack.Top();
    for (int neighbor : Graph.GetNeighbors(currentVertex)) {
        if (DiscoveredVertices.Contains(neighbor)) {
            continue;
        }
        DiscoveredVertices.Add(neighbor);
        TraversalStack.Add(neighbor);
        GoToVertex(neighbor);
        return true;
    }
    TraversalStack.Pop();
    if (TraversalStack.Num() > 0) {
        GoToVertex(TraversalStack.Top());
        return true;
    }

    return false;
}

float ACleverAIController::GetDirectionScale(FVector direction)
{
    auto currentLocation = GetCharacterLocation();
    float scale = InitialScale;
    while (!CheckVisibility(currentLocation + scale * direction)) {
        scale *= ScaleDecayRate;
    }
    return scale;
}

void ACleverAIController::DiscoverNeighborhood()
{
    auto currentLocation = GetCharacterLocation();
    for (float angle = 0.0f; angle <= 2 * PI; angle += 2 * PI / ChooseDirectionProbeCount) {
        auto direction = UStaticLibrary::GetAngleDirection(angle);
        auto scale = GetDirectionScale(direction);
        if (scale < 2 * MinAllowedScale) {
            continue;
        }
        scale = MinAllowedScale;
        auto nextLocation = currentLocation + direction * scale;
        auto closeVertices = Graph.FindCloseVertices(nextLocation, MinAllowedScale / 2);
        if (closeVertices.Num() == 0) {
            int nextVertex = Graph.AddVertex(nextLocation);
            Graph.AddEdge(CurrentVertex, nextVertex, scale);
        }
    }
}
