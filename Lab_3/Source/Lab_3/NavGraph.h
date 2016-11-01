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

    bool IsVisited(int index) const;

    void Visit(int index);

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
        bool bVisited = false;
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
    FColor VertexColor = FColor(255, 0, 0);
    FColor EdgeColor = FColor(255, 0, 0);
    FColor DiscoveryColor = FColor(0, 0, 255);

};
