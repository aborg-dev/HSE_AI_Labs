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

    int AddVertex(const FVector& vertex);

    int FindVertex(const FVector& vertex);

    TArray<int> GetNeighbors(int index);

    FVector GetVertexByIndex(int index);

    TArray<std::pair<int, FVector>> FindCloseVertices(const FVector& vertex, float distance);

    void AddEdge(int first, int second, float distance);

private:
    TArray<FVector> Vertices;

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

    bool ValidateVertexIndex(int index);

};
