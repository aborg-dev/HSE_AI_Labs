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

    Graph = nullptr;
}

void ACleverAIController::BeginPlay()
{
    Super::BeginPlay();

    auto* controller = Cast<ACleverAIController>(GetControllerById(0));
    if (ControllerId == 0 || !controller) {
        Graph = MakeShareable(new NavGraph());
        Graph->SetWorld(GetWorld());
    } else {
        Graph = controller->GetNavigationGraph();
    }
}

void ACleverAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentVertex == NO_VERTEX) {
        CurrentVertex = Graph->AddVertex(GetCharacterLocation());
        TraversalStack.Add(CurrentVertex);
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
                NextVertex = Graph->AddVertex(currentLocation);
                Graph->AddEdge(
                    CurrentVertex,
                    NextVertex,
                    (Graph->GetVertexByIndex(CurrentVertex) - currentLocation).Size());
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

    if (!Graph->IsVisited(CurrentVertex)) {
        DiscoverNeighborhood();
    }
    if (!ChooseDirection()) {
        UE_LOG(LogTemp, Warning, TEXT("Traversal completed"));
    }
}

int ACleverAIController::GetCurrentVertex()
{
    return CurrentVertex;
}

TSharedPtr<NavGraph> ACleverAIController::GetNavigationGraph()
{
    return Graph;
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
    Graph->Visit(currentVertex);
    for (int neighbor : Graph->GetNeighbors(currentVertex)) {
        if (Graph->IsVisited(neighbor)) {
            continue;
        }
        GoToVertex(Graph->GetVertexByIndex(neighbor));
        return true;
    }

    if (Graph->HasPossibleDiscoveries(currentVertex)) {
        auto discovery = Graph->GetAndPopOneDiscovery(currentVertex);
        GoToVertex(discovery);
        return true;
    }

    TraversalStack.Pop();
    if (TraversalStack.Num() > 0) {
        UE_LOG(LogTemp, Warning, TEXT("Return through traversal stack to vertex %d"), TraversalStack.Top());
        bIsMovingBack = true;
        NextVertex = TraversalStack.Top();
        GoToVertex(Graph->GetVertexByIndex(TraversalStack.Top()));
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
        auto closeVertices = Graph->FindCloseVertices(nextLocation, MinAllowedScale / 2);
        if (closeVertices.Num() == 0) {
            UE_LOG(LogTemp, Warning, TEXT("Adding discovery %s -> %s"),
                *currentLocation.ToString(),
                *nextLocation.ToString());
            Graph->AddPossibleDiscovery(CurrentVertex, nextLocation);
        }
    }

    for (int id = 0; id < GetControllerCount(); ++id) {
        if (id == ControllerId) {
            continue;
        }
        auto* controller = Cast<ACleverAIController>(GetControllerById(id));
        if (controller) {
            auto location = controller->GetCharacterLocation();
            auto vertex = controller->GetCurrentVertex();
            auto distance = (currentLocation - location).Size();
            if (vertex == NO_VERTEX || distance > MinAllowedScale) {
                continue;
            }
            Graph->AddEdge(CurrentVertex, vertex, distance);
        }
    }
}
