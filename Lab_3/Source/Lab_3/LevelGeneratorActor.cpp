// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "LevelGeneratorActor.h"
#include "Lab_3GameMode.h"

#include <functional>

// Sets default values
ALevelGeneratorActor::ALevelGeneratorActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    RootComponent = FloorMesh;

    WallActorCount = 3;
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
    GridOrigin.X = FloorOrigin.X - FloorBoxExtent.X;
    GridOrigin.Y = FloorOrigin.Y - FloorBoxExtent.Y;
    GridOrigin.Z = FloorOrigin.Z + FloorBoxExtent.Z;
    UE_LOG(LogTemp, Warning, TEXT("FloorOrigin: %s"), *FloorOrigin.ToString());
    UE_LOG(LogTemp, Warning, TEXT("FloorBoxExtent: %s"), *FloorBoxExtent.ToString());
    UE_LOG(LogTemp, Warning, TEXT("GridOrigin: %s"), *GridOrigin.ToString());

    if (WallActor) {
        UWorld* const World = GetWorld();
        if (World) {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = Instigator;
            FVector SpawnLocation(0.f, 0.f, 0.f);
            FRotator SpawnRotation(0.f, 0.f, 0.f);
            AActor* Actor = World->SpawnActor<AActor>(WallActor, SpawnLocation, SpawnRotation, SpawnParams);
            Actor->GetActorBounds(true, SpawnLocation, WallActorBoxExtent);
            Actor->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("WallActorBoxExtent: %s"), *WallActorBoxExtent.ToString());

            CellHeight = WallActorBoxExtent.X * 2;
            CellWidth = WallActorBoxExtent.Y * 2;
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
    if (WallActor) {
        GenerateMaze();
        SpawnMaze();
        /*
        for (int i = 0; i < WallActorCount; ++i) {
            auto Cell = GenerateRandomCell();
            GridOccupied[Cell.X][Cell.Y] = true;
            SpawnWall(GetCellLocation(Cell));
        }
        */
    }
}

const int DX[4] = {1, 0, -1, 0};
const int DY[4] = {0, 1, 0, -1};

bool ALevelGeneratorActor::IsBorderCell(int row, int column) const
{
    return row == 0 || row == GridRows - 1
        || column == 0 || column == GridColumns - 1;
}

bool ALevelGeneratorActor::IsValidCell(int row, int column) const
{
    return row >= 0 && row < GridRows
        && column >= 0 && column < GridColumns;
}

bool ALevelGeneratorActor::HasOccupiedNeighbors(int row, int column) const
{
    for (int dRow = -1; dRow <= 1; ++dRow) {
        for (int dColumn = -1; dColumn <= 1; ++dColumn) {
            int nRow = row + dRow;
            int nColumn = column + dColumn;
            if (IsValidCell(nRow, nColumn) && GridOccupied[nRow][nColumn]) {
                return true;
            }
        }
    }
    return false;
}

void ALevelGeneratorActor::GenerateMaze()
{
    for (int row = 0; row < GridRows; ++row) {
        for (int column = 0; column < GridColumns; ++column) {
            GridOccupied[row][column] = true;
        }
    }

    std::function<void(int, int)> Walk = [&] (int row, int column) {
        GridOccupied[row][column] = false;
        while (true) {
            TArray<int> validDirections;
            for (int dir = 0; dir < 4; ++dir) {
                int nRow = row + DX[dir];
                int nColumn = column + DY[dir];
                int nnRow = row + 2 * DX[dir];
                int nnColumn = column + 2 * DY[dir];
                if (!IsValidCell(nnRow, nnColumn) || !GridOccupied[nnRow][nnColumn]) {
                    continue;
                }
                if (IsBorderCell(nnRow, nnColumn)) {
                    continue;
                }
                validDirections.Add(dir);
            }
            if (validDirections.Num() == 0) {
                break;
            }
            int dirId = RandomStream.RandRange(0, validDirections.Num() - 1);
            int dir = validDirections[dirId];
            int nRow = row + DX[dir];
            int nColumn = column + DY[dir];
            GridOccupied[nRow][nColumn] = false;
            int nnRow = row + 2 * DX[dir];
            int nnColumn = column + 2 * DY[dir];
            Walk(nnRow, nnColumn);
        }
    };

    Walk(1, 1);
}

void ALevelGeneratorActor::SpawnMaze()
{
    FString Maze;
    for (int row = 0; row < GridRows; ++row) {
        for (int column = 0; column < GridColumns; ++column) {
            if (GridOccupied[row][column]) {
                Maze += '#';
                FIntVector Cell(row, column, 0);
                SpawnWall(GetCellLocation(Cell));
            } else {
                Maze += '*';
            }
        }
        Maze += '\n';
    }
    UE_LOG(LogTemp, Warning, TEXT("\n%s"), *Maze);
}

FIntVector ALevelGeneratorActor::GenerateRandomCell() const
{
    int RandomRow;
    int RandomColumn;
    do {
        RandomRow = RandomStream.RandRange(0, GridRows - 1);
        RandomColumn = RandomStream.RandRange(0, GridColumns - 1);
    } while (GridOccupied[RandomRow][RandomColumn]);

    return FIntVector(RandomRow, RandomColumn, 0);
}

FVector ALevelGeneratorActor::GetCellLocation(FIntVector Cell) const
{
    FVector Location;
    Location.X = GridOrigin.X + Cell.X * CellHeight;
    Location.Y = GridOrigin.Y + Cell.Y * CellWidth;
    Location.Z = GridOrigin.Z;
    return Location;
}

void ALevelGeneratorActor::SpawnWall(FVector SpawnLocation)
{
    UWorld* const World = GetWorld();
    if (!World) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Instigator;

    FRotator SpawnRotation(0.f, 0.f, 0.f);

    UE_LOG(LogTemp, Warning, TEXT("Spawned wall actor at %s"), *SpawnLocation.ToString());
    AActor* Actor = World->SpawnActor<AActor>(WallActor, SpawnLocation, SpawnRotation, SpawnParams);
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

    auto* MyGameMode = Cast<ALab_3GameMode>(UGameplayStatics::GetGameMode(this));
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

