// Fill out your copyright notice in the Description page of Project Settings.

#include "Lesson_1.h"
#include "SpawnVolume.h"
#include "Pickup.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    WhereToSpawn = CreateDefaultSubobject<UBoxComponent>(TEXT("WhereToSpawn"));
    RootComponent = WhereToSpawn;

    SpawnDelayRangeLow = 1.0f;
    SpawnDelayRangeHigh = 4.5f;
    SpawnDelay = GetRandomSpawnDelay();
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnVolume::Tick( float DeltaTime )
{
    if (!bSpawningEnabled) {
        return;
    }

	Super::Tick( DeltaTime );

    SpawnTime += DeltaTime;
    bool bShouldSpawn = SpawnTime > SpawnDelay;
    if (bShouldSpawn) {
        SpawnPickup();
        SpawnTime -= SpawnDelay;
        SpawnDelay = GetRandomSpawnDelay();
    }
}

void ASpawnVolume::SpawnPickup()
{
    if (WhatToSpawn) {
        UWorld* const World = GetWorld();
        if (World) {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = Instigator;

            FVector SpawnLocation = GetRandomPointInVolume();
            FRotator SpawnRotation;
            SpawnRotation.Yaw = FMath::FRand() * 360.f;
            SpawnRotation.Pitch = FMath::FRand() * 360.f;
            SpawnRotation.Roll = FMath::FRand() * 360.f;

            APickup* const SpawnedPickup = World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
        }
    }
}

float ASpawnVolume::GetRandomSpawnDelay()
{
    return FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
}

FVector ASpawnVolume::GetRandomPointInVolume()
{
    FVector RandomLocation;
    float MinX, MinY, MinZ;
    float MaxX, MaxY, MaxZ;

    FVector Origin;
    FVector BoxExtent;

    // Get the SpawnVolume's origin and box extent
    Origin = WhereToSpawn->Bounds.Origin;
    BoxExtent = WhereToSpawn->Bounds.BoxExtent;

    // Calculate the minimum X, Y, and Z
    MinX = Origin.X - BoxExtent.X / 2.f;
    MinY = Origin.Y - BoxExtent.Y / 2.f;
    MinZ = Origin.Z - BoxExtent.Z / 2.f;

    // Calculate the maximum X, Y, and Z
    MaxX = Origin.X + BoxExtent.X / 2.f;
    MaxY = Origin.Y + BoxExtent.Y / 2.f;
    MaxZ = Origin.Z + BoxExtent.Z / 2.f;

    // The random spawn location will fall between the min and max X, Y, and Z
    RandomLocation.X = FMath::FRandRange(MinX, MaxX);
    RandomLocation.Y = FMath::FRandRange(MinY, MaxY);
    RandomLocation.Z = FMath::FRandRange(MinZ, MaxZ);
    
    // Return the random spawn location
    return RandomLocation;
}

void ASpawnVolume::EnableSpawning()
{
    bSpawningEnabled = true;
}

void ASpawnVolume::DisableSpawning()
{
    bSpawningEnabled = false;
}