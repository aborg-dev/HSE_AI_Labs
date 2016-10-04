// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class LESSON_1_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    // Box that represents the volume where random spawning will happen.
    UPROPERTY(VisibleInstanceOnly, Category = Spawning)
    UBoxComponent* WhereToSpawn;

    // Type of pickup that will be spawned in the volume.
    UPROPERTY(EditAnywhere, Category = Spawning)
    TSubclassOf<class APickup> WhatToSpawn;

    // Minimum spawning delay.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawning)
    float SpawnDelayRangeLow;

    // Maximum spawning delay.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawning)
    float SpawnDelayRangeHigh;

    // Returns a random point that is located in box specified by WhereToSpawn.
    UFUNCTION(BlueprintPure, Category = Spawning)
    FVector GetRandomPointInVolume();

    // Enables spawning.
    void EnableSpawning();

    // Disables spawning.
    void DisableSpawning();

private:

    // True if spawning is enabled, false otherwise.
    bool bSpawningEnabled;

    // Returns a random float in the interval [SpawnDelayRangeLow, SpawnDelayRangeHigh].
    float GetRandomSpawnDelay();

    // Stores spawn delay for current episode.
    float SpawnDelay;

    // Timer for the spawn of the pickup.
    float SpawnTime;

    // Spawns pickup at random location.
    void SpawnPickup();
};
