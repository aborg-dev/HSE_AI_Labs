// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "CleverAIController.h"
#include "StaticLibrary.h"

const int NO_VERTEX = -1;

ACleverAIController::ACleverAIController()
{
    ChooseDirectionProbeCount = 4;
    MinAllowedScale = 100.0f;
    InitialScale = 300.0f;
    ScaleDecayRate = 0.9f;
    bIsMoving = false;
    AcceptableDistanceToTarget = 20.0f;
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
        auto distance = (currentLocation - NextVertexLocation).Size();
        UE_LOG(LogTemp, Warning, TEXT("Current location: %s, Next location: %s"),
            *currentLocation.ToString(),
            *NextVertexLocation.ToString());
        UE_LOG(LogTemp, Warning, TEXT("Distance to target: %.3f"), distance);
        if (previousDistanceToNextVertex - distance < 0.1) {
            bIsMoving = false;
            if (!bIsMovingBack) {
                NextVertexLocation = currentLocation;
                NextVertex = Graph.AddVertex(currentLocation);
                Graph.AddEdge(
                    CurrentVertex,
                    NextVertex,
                    (Graph.GetVertexByIndex(CurrentVertex) - currentLocation).Size());
                DiscoveredVertices.Add(NextVertex);
                TraversalStack.Add(NextVertex);
            }
            CurrentVertex = NextVertex;
            NextVertex = NO_VERTEX;
            previousDistanceToNextVertex = 1e9;
        } else {
            previousDistanceToNextVertex = distance;
            return;
        }
    }

    if (!VisitedVertices.Contains(CurrentVertex)) {
        DiscoverNeighborhood();
    }
    if (!ChooseDirection()) {
        UE_LOG(LogTemp, Warning, TEXT("Traversal completed"));
    }
}

void ACleverAIController::GoToVertex(FVector vertexLocation)
{
    NextVertexLocation = vertexLocation;
    bIsMoving = true;
    UE_LOG(LogTemp, Warning, TEXT("Going to vertex %s"),
        *NextVertexLocation.ToString());
    SetNewMoveDestination(NextVertexLocation);
}

bool ACleverAIController::ChooseDirection()
{
    if (TraversalStack.Num() == 0) {
        return false;
    }

    bIsMovingBack = false;
    int currentVertex = TraversalStack.Top();
    UE_LOG(LogTemp, Warning, TEXT("ChooseDirection: currentVertex: %d"), currentVertex);
    VisitedVertices.Add(currentVertex);
    for (int neighbor : Graph.GetNeighbors(currentVertex)) {
        if (DiscoveredVertices.Contains(neighbor)) {
            continue;
        }
        GoToVertex(Graph.GetVertexByIndex(neighbor));
        return true;
    }

    auto* discoveries = PossibleDiscoveries.Find(currentVertex);
    if (discoveries) {
        UE_LOG(LogTemp, Warning, TEXT("ChooseDirection: Possible discoveries num: %d"), discoveries->Num());
        if (discoveries->Num() > 0) {
            auto target = discoveries->Top();
            discoveries->Pop();
            GoToVertex(target);
            return true;
        }
    }

    TraversalStack.Pop();
    if (TraversalStack.Num() > 0) {
        UE_LOG(LogTemp, Warning, TEXT("Return through traversal stack to vertex %d"), TraversalStack.Top());
        bIsMovingBack = true;
        NextVertex = TraversalStack.Top();
        GoToVertex(Graph.GetVertexByIndex(TraversalStack.Top()));
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
        //UE_LOG(LogTemp, Warning, TEXT("Direction: %s, scale: %.3f"),
            //*direction.ToString(),
            //scale);
        scale = MinAllowedScale;
        auto nextLocation = currentLocation + direction * scale;
        auto closeVertices = Graph.FindCloseVertices(nextLocation, MinAllowedScale / 2);
        if (closeVertices.Num() == 0) {
            UE_LOG(LogTemp, Warning, TEXT("Adding discovery %s -> %s"),
                *currentLocation.ToString(),
                *nextLocation.ToString());
            PossibleDiscoveries.FindOrAdd(CurrentVertex).Add(nextLocation);
        }
    }
}
