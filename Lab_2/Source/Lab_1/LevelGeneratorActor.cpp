// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "LevelGeneratorActor.h"
#include "Kismet/GameplayStatics.h"

#include "Lab_1GameMode.h"
#include "HouseActor.h"

// Sets default values
ALevelGeneratorActor::ALevelGeneratorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    RootComponent = FloorMesh;

    HouseActorCount = 3;
    RandomSeed = 42;
    EnableGeneration = false;
}

void ALevelGeneratorActor::OnConstruction(const FTransform& Transform)
{
    if (EnableGeneration) {
        RandomStream.Initialize(RandomSeed);
        CollectWorldParameters();
        DeleteOldActors();
        SpawnNewActors();
    }
}

void ALevelGeneratorActor::CollectWorldParameters()
{
    FloorOrigin = FloorMesh->Bounds.Origin;
    FloorBoxExtent = FloorMesh->Bounds.BoxExtent;
    GridOrigin.X = FloorOrigin.X - FloorBoxExtent.X * 0.9;
    GridOrigin.Y = FloorOrigin.Y - FloorBoxExtent.Y * 0.9;
    GridOrigin.Z = FloorOrigin.Z + FloorBoxExtent.Z * 0.9;
    UE_LOG(LogTemp, Warning, TEXT("FloorOrigin: %s"), *FloorOrigin.ToString());
    UE_LOG(LogTemp, Warning, TEXT("FloorBoxExtent: %s"), *FloorBoxExtent.ToString());
    UE_LOG(LogTemp, Warning, TEXT("GridOrigin: %s"), *GridOrigin.ToString());

    if (HouseActor) {
        UWorld* const World = GetWorld();
        if (World) {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = Instigator;
            FVector SpawnLocation(0.f, 0.f, 0.f);
            FRotator SpawnRotation(0.f, 0.f, 0.f);
            AHouseActor* Actor = World->SpawnActor<AHouseActor>(HouseActor, SpawnLocation, SpawnRotation, SpawnParams);
            Actor->GetActorBounds(true, SpawnLocation, HouseActorBoxExtent);
            Actor->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("HouseActorBoxExtent: %s"), *HouseActorBoxExtent.ToString());

            CellHeight = HouseActorBoxExtent.X * 1.1;
            CellWidth = HouseActorBoxExtent.Y * 1.1;
            GridRows = FloorBoxExtent.X * 2 / CellHeight;
            GridColumns = FloorBoxExtent.Y * 2 / CellWidth;
            UE_LOG(LogTemp, Warning, TEXT("Grid rows: %d, Grid columns: %d"), GridRows, GridColumns);
        }
    }
    GridOccupied.SetNum(GridRows);
    for (int i = 0; i < GridRows; ++i) {
        GridOccupied[i].SetNumZeroed(GridColumns);
    }
}

void ALevelGeneratorActor::DeleteOldActors()
{
    for (auto* Actor : SpawnedActors) {
        Actor->Destroy();
    }
    SpawnedActors.Empty();
}

void ALevelGeneratorActor::SpawnNewActors()
{
    if (HouseActor) {
        for (int i = 0; i < HouseActorCount; ++i) {
            SpawnHouse();
        }
    }
}

FVector ALevelGeneratorActor::GenerateRandomLocation()
{
    // The random spawn location will fall between the min and max X, Y, and Z
    int RandomRow;
    int RandomColumn;
    do {
        RandomRow = RandomStream.RandRange(0, GridRows - 1);
        RandomColumn = RandomStream.RandRange(0, GridColumns - 1);
    } while (GridOccupied[RandomRow][RandomColumn]);

    GridOccupied[RandomRow][RandomColumn] = true;

    FVector RandomLocation;
    RandomLocation.X = GridOrigin.X + RandomRow * CellHeight;
    RandomLocation.Y = GridOrigin.Y + RandomColumn * CellWidth;
    RandomLocation.Z = GridOrigin.Z;

    // Return the random spawn location
    return RandomLocation;
}

void ALevelGeneratorActor::SpawnHouse()
{
    UWorld* const World = GetWorld();
    if (!World) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Instigator;

    FVector SpawnLocation(GenerateRandomLocation());
    FRotator SpawnRotation(0.f, 0.f, 0.f);

    UE_LOG(LogTemp, Warning, TEXT("Spawned house actor at %s"), *SpawnLocation.ToString());
    AHouseActor* Actor = World->SpawnActor<AHouseActor>(HouseActor, SpawnLocation, SpawnRotation, SpawnParams);
    FVector Origin;
    FVector BoxExtent;
    Actor->GetActorBounds(true, Origin, BoxExtent);
    FVector Location = Actor->GetActorLocation();
    Location.Z += BoxExtent.Z;
    Actor->SetActorLocation(Location);
    SpawnedActors.Add(Actor);
}

// Called when the game starts or when spawned
void ALevelGeneratorActor::BeginPlay()
{
    Super::BeginPlay();

    auto* MyGameMode = Cast<ALab_1GameMode>(UGameplayStatics::GetGameMode(this));
    if (MyGameMode) {
        MyGameMode->SetWorldOrigin(FloorOrigin);
        FVector WorldSize = FloorBoxExtent;
        WorldSize.X *= 2;
        WorldSize.Y *= 2;
        MyGameMode->SetWorldSize(WorldSize);
    }
}

// Called every frame
void ALevelGeneratorActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

