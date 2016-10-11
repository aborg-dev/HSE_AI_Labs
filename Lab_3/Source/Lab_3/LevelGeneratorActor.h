// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LevelGeneratorActor.generated.h"

UCLASS()
class LAB_3_API ALevelGeneratorActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALevelGeneratorActor();

    void OnConstruction(const FTransform& Transform) override;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick( float DeltaSeconds ) override;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Level)
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(EditAnywhere, Category = Spawning)
    TSubclassOf<class AActor> WallActor;

    // Number of house actors to spawn.
    UPROPERTY(EditAnywhere, Category = Spawning)
    int WallActorCount;

    UPROPERTY(EditAnywhere, Category = Spawning)
    int RandomSeed;

    UPROPERTY(EditAnywhere, Category = Spawning)
    bool EnableGeneration;

private:
    void DeleteOldActors();
    void SpawnNewActors();

    void CollectWorldParameters();

    void GenerateMaze();

    void SpawnMaze();

    void SpawnWall(FVector SpawnLocation);

    FIntVector GenerateRandomCell() const;

    FVector GetCellLocation(FIntVector Cell) const;

    bool IsBorderCell(int row, int column) const;

    bool IsValidCell(int row, int column) const;

    bool HasOccupiedNeighbors(int row, int column) const;

    TArray<AActor*> SpawnedActors;

    FRandomStream RandomStream;

    FVector FloorOrigin;
    FVector FloorBoxExtent;

    FVector WallActorBoxExtent;

    TArray<TArray<bool>> GridOccupied;

    FVector GridOrigin;
    float CellHeight;
    float CellWidth;
    int GridRows, GridColumns;

};

