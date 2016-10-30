// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "CleverAIController.h"
#include "StaticLibrary.h"
#include "MazeExit.h"

const int NO_VERTEX = -1;

ACleverAIController::ACleverAIController()
{
    ChooseDirectionProbeCount = 4;
    MinAllowedScale = 150.0f;
    InitialScale = 300.0f;
    ScaleDecayRate = 0.9f;
    bIsMoving = false;
    AcceptableDistanceToTarget = 10.0f;
    CurrentVertex = NO_VERTEX;
    MinAllowedSpeed = 50.f;
    OvershootDistance = 30.0f;

    PreviousDistanceToNextVertex = MinAllowedScale;

    MinTryMoveTime = 0.5f;
    TryMoveTime = 0.0f;

    Graph.SetWorld(GetWorld());
}

void ACleverAIController::BeginPlay()
{
    Super::BeginPlay();

    switch (ControllerId) {
        case 0 : Graph.SetColor(FColor(255, 0, 0)); break;
        case 1 : Graph.SetColor(FColor(0, 255, 0)); break;
        case 2 : Graph.SetColor(FColor(0, 0, 255)); break;
    }
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentVertex == NO_VERTEX) {
        CurrentVertex = Graph.AddVertex(GetCharacterLocation());
        TraversalStack.Add(CurrentVertex);
        DiscoveredVertices.Add(CurrentVertex);
    }

    if (TryEscape()) {
        return;
    }

    if (bIsMoving) {
        auto currentLocation = GetCharacterLocation();
        auto distance = (currentLocation - NextVertexLocation).Size();
        float speed = fabs(PreviousDistanceToNextVertex - distance) / DeltaSeconds;
        UE_LOG(LogTemp, Warning, TEXT("Current location: %s, Next location: %s, distance: %.3f, speed: %.3f"),
            *currentLocation.ToString(),
            *NextVertexLocation.ToString(),
            distance,
            speed);

        TryMoveTime += DeltaSeconds;
        if (distance < AcceptableDistanceToTarget || (speed < MinAllowedSpeed && TryMoveTime > MinTryMoveTime)) {
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
            PreviousDistanceToNextVertex = MinAllowedScale;
            TryMoveTime = 0.0f;
        } else {
            PreviousDistanceToNextVertex = distance;
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

bool ACleverAIController::TryEscape()
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

void ACleverAIController::GoToVertex(FVector vertexLocation)
{
    NextVertexLocation = vertexLocation;
    bIsMoving = true;
    UE_LOG(LogTemp, Warning, TEXT("Going to vertex %s"),
        *NextVertexLocation.ToString());

    UKismetSystemLibrary::DrawDebugLine(
        GetWorld(),
        GetCharacterLocation(),
        vertexLocation,
        FColor(0, 255, 0),
        10.f);

    auto delta = NextVertexLocation - GetCharacterLocation();
    delta /= delta.Size();
    SetNewMoveDestination(NextVertexLocation + delta * OvershootDistance);
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
    for (int i = 0; i < ChooseDirectionProbeCount; ++i) {
        float angle = i * (2 * PI / ChooseDirectionProbeCount);
        auto direction = UStaticLibrary::GetAngleDirection(angle);
        auto scale = GetDirectionScale(direction);
        if (scale < MinAllowedScale) {
            continue;
        }
        UE_LOG(LogTemp, Warning, TEXT("Direction: %s, scale: %.3f, angle: %.3f"),
            *direction.ToString(),
            scale,
            angle);
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
