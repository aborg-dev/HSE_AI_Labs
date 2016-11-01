// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "NavGraph.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

NavGraph::NavGraph()
{
}

NavGraph::~NavGraph()
{
}

void NavGraph::SetWorld(UWorld* world)
{
    World = world;
}

void NavGraph::SetColor(FColor color)
{
    VertexColor = EdgeColor = color;
}

int NavGraph::AddVertex(const FVector& vertex)
{
    if (World) {
        UKismetSystemLibrary::DrawDebugPoint(
            World,
            vertex,
            3,
            VertexColor,
            300.f);
    }

    Vertices.Emplace(vertex);
    Edges.Emplace();
    return Vertices.Num() - 1;
}

int NavGraph::FindVertex(const FVector& vertex)
{
    for (int i = 0; i < Vertices.Num(); ++i) {
        if (Vertices[i].Location == vertex) {
            return i;
        }
    }
    return NOT_FOUND;
}

void NavGraph::AddPossibleDiscovery(int index, const FVector& discovery)
{
    ValidateVertexIndex(index);

    if (World) {
        UKismetSystemLibrary::DrawDebugPoint(
            World,
            discovery,
            3,
            DiscoveryColor,
            300.f);

        UKismetSystemLibrary::DrawDebugLine(
            World,
            GetVertexByIndex(index),
            discovery,
            DiscoveryColor,
            300.f);
    }

    Vertices[index].PossibleDiscoveries.Add(discovery);
}

bool NavGraph::HasPossibleDiscoveries(int index) const
{
    ValidateVertexIndex(index);

    return Vertices[index].PossibleDiscoveries.Num() > 0;
}

FVector NavGraph::GetAndPopOneDiscovery(int index)
{
    ValidateVertexIndex(index);

    auto& discoveries = Vertices[index].PossibleDiscoveries;
    if (discoveries.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Trying to pop from empty discoveries list of %d"), index);
        return FVector(0, 0, 0);
    }

    auto result = discoveries.Top();
    discoveries.Pop();
    return result;
}

TArray<int> NavGraph::GetNeighbors(int index)
{
    ValidateVertexIndex(index);

    TArray<int> result;
    for (const auto& edge : Edges[index]) {
        result.Emplace(edge.To);
    }
    return result;
}

void NavGraph::ValidateVertexIndex(int index) const
{
    ensureMsgf(index < Vertices.Num(),
        TEXT("Trying to extract non-existent vertex %d"), index);
}

FVector NavGraph::GetVertexByIndex(int index)
{
    ValidateVertexIndex(index);

    return Vertices[index].Location;
}

void NavGraph::AddEdge(int first, int second, float distance)
{
    ValidateVertexIndex(first);
    ValidateVertexIndex(second);

    Edges[first].Emplace(second, distance);
    Edges[second].Emplace(first, distance);

    if (World) {
        UKismetSystemLibrary::DrawDebugLine(
            World,
            GetVertexByIndex(first),
            GetVertexByIndex(second),
            EdgeColor,
            300.f);
    }
}

TArray<std::pair<int, FVector>> NavGraph::FindCloseVertices(const FVector& vertex, float distance) {
    TArray<std::pair<int, FVector>> result;
    for (int i = 0; i < Vertices.Num(); ++i) {
        float curDistance = (Vertices[i].Location - vertex).Size();
        if (curDistance < distance) {
            result.Emplace(i, Vertices[i].Location);
        }
    }
    return result;
}
