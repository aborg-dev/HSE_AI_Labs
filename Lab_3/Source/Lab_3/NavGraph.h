// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>

const int NOT_FOUND = -1;

/**
 * 
 */
class LAB_3_API NavGraph
{
public:
    NavGraph();
    ~NavGraph();

    void SetWorld(UWorld* world);

    void SetColor(FColor color);

    int AddVertex(const FVector& vertex);

    int FindVertex(const FVector& vertex);

    void AddPossibleDiscovery(int index, const FVector& discovery);

    bool HasPossibleDiscoveries(int index) const;

    FVector GetAndPopOneDiscovery(int index);

    TArray<int> GetNeighbors(int index);

    FVector GetVertexByIndex(int index);

    TArray<std::pair<int, FVector>> FindCloseVertices(const FVector& vertex, float distance);

    void AddEdge(int first, int second, float distance);

private:
    struct Vertex
    {
        Vertex(FVector location)
            : Location(location)
        { }

        Vertex()
            : Location(FVector(0, 0, 0))
        { }

        FVector Location;
        TArray<FVector> PossibleDiscoveries;
    };

    TArray<Vertex> Vertices;

    struct Edge
    {
        Edge(int to, float distance)
            : To(to)
            , Distance(distance)
        { }

        Edge() {}

        int To;
        float Distance;
    };

    TArray<TArray<Edge>> Edges;

    void ValidateVertexIndex(int index) const;

    UWorld* World = nullptr;
    FColor Color = FColor(255, 0, 0);

};
