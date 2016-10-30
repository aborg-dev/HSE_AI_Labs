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
    Color = color;
}

int NavGraph::AddVertex(const FVector& vertex)
{
    if (World) {
        UKismetSystemLibrary::DrawDebugPoint(
            World,
            vertex,
            3,
            Color,
            300.f);
    }

    Vertices.Add(vertex);
    Edges.Emplace();
    return Vertices.Num() - 1;
}

int NavGraph::FindVertex(const FVector& vertex)
{
    for (int i = 0; i < Vertices.Num(); ++i) {
        if (Vertices[i] == vertex) {
            return i;
        }
    }
    return NOT_FOUND;
}

TArray<int> NavGraph::GetNeighbors(int index)
{
    TArray<int> result;
    if (!ValidateVertexIndex(index)) {
        return result;
    }
    for (const auto& edge : Edges[index]) {
        result.Emplace(edge.To);
    }
    return result;
}

bool NavGraph::ValidateVertexIndex(int index)
{
    if (index >= Vertices.Num()) {
        UE_LOG(LogTemp, Warning, TEXT("Trying to add edge to non-existent vertex %d"), index);
        return false;
    }
    return true;
}

FVector NavGraph::GetVertexByIndex(int index)
{
    if (!ValidateVertexIndex(index)) {
        return FVector(0, 0, 0);
    }
    return Vertices[index];
}

void NavGraph::AddEdge(int first, int second, float distance)
{
    if (!ValidateVertexIndex(first) || !ValidateVertexIndex(second)) {
        return;
    }
    Edges[first].Emplace(second, distance);
    Edges[second].Emplace(first, distance);

    if (World) {
        UKismetSystemLibrary::DrawDebugLine(
            World,
            GetVertexByIndex(first),
            GetVertexByIndex(second),
            Color,
            300.f);
    }
}

TArray<std::pair<int, FVector>> NavGraph::FindCloseVertices(const FVector& vertex, float distance) {
    TArray<std::pair<int, FVector>> result;
    for (int i = 0; i < Vertices.Num(); ++i) {
        float curDistance = (Vertices[i] - vertex).Size();
        if (curDistance < distance) {
            result.Emplace(i, Vertices[i]);
        }
    }
    return result;
}
