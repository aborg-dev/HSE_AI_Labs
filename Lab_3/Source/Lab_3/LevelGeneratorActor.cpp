// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "LevelGeneratorActor.h"
#include "Lab_3GameMode.h"
#include "MazeExit.h"

#include <functional>
#include <numeric>

// Sets default values
ALevelGeneratorActor::ALevelGeneratorActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    RootComponent = FloorMesh;

    CharacterActorCount = 1;
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
    Grid.SetNum(GridRows);
    for (int i = 0; i < GridRows; ++i) {
        Grid[i].Init(GridContent::NONE, GridColumns);
    }
}

void ALevelGeneratorActor::DeleteOldActors()
{
    for (auto* Actor : SpawnedActors) {
        if (Actor && Actor->IsValidLowLevel()) {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();
}

void ALevelGeneratorActor::SpawnNewActors()
{
    if (WallActor && ExitActor && CharacterActor) {
        GenerateMaze();
        SpawnMaze();
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
            if (IsValidCell(nRow, nColumn) && Grid[nRow][nColumn] != GridContent::NONE) {
                return true;
            }
        }
    }
    return false;
}

bool ALevelGeneratorActor::HasFreeNeighbors(int row, int column) const
{
    for (int dir = 0; dir < 4; ++dir) {
        int nRow = row + DX[dir];
        int nColumn = column + DY[dir];
        if (IsValidCell(nRow, nColumn) && Grid[nRow][nColumn] == GridContent::NONE) {
            return true;
        }
    }
    return false;
}

void ALevelGeneratorActor::GenerateWalls()
{
    for (int row = 0; row < GridRows; ++row) {
        for (int column = 0; column < GridColumns; ++column) {
            Grid[row][column] = GridContent::WALL;
        }
    }

    std::function<void(int, int)> Walk = [&] (int row, int column) {
        Grid[row][column] = GridContent::NONE;
        while (true) {
            TArray<int> validDirections;
            for (int dir = 0; dir < 4; ++dir) {
                int nRow = row + DX[dir];
                int nColumn = column + DY[dir];
                int nnRow = row + 2 * DX[dir];
                int nnColumn = column + 2 * DY[dir];
                if (!IsValidCell(nnRow, nnColumn) || Grid[nnRow][nnColumn] == GridContent::NONE) {
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
            Grid[nRow][nColumn] = GridContent::NONE;
            int nnRow = row + 2 * DX[dir];
            int nnColumn = column + 2 * DY[dir];
            Walk(nnRow, nnColumn);
        }
    };

    Walk(1, 1);
}

void ALevelGeneratorActor::GenerateExits()
{
    // Generate exits
    TArray<std::pair<int, int>> candidates;
    for (int row = 0; row < GridRows; ++row) {
        for (int column = 0; column < GridColumns; ++column) {
            if (IsBorderCell(row, column)
                && Grid[row][column] == GridContent::WALL
                && HasFreeNeighbors(row, column))
            {
                candidates.Add(std::make_pair(row, column));
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Wall candidates number: %d"), candidates.Num());

    if (candidates.Num() == 0) {
        return;
    }

    int candidateId = RandomStream.RandRange(0, candidates.Num() - 1);
    auto candidate = candidates[candidateId];
    Grid[candidate.first][candidate.second] = GridContent::EXIT;
}

void ALevelGeneratorActor::GenerateCharacters()
{
    for (int i = 0; i < CharacterActorCount; ++i) {
        TArray<std::pair<int, int>> candidates;
        for (int row = 0; row < GridRows; ++row) {
            for (int column = 0; column < GridColumns; ++column) {
                if (Grid[row][column] == GridContent::NONE) {
                    candidates.Add(std::make_pair(row, column));
                }
            }
        }
        int candidateId = RandomStream.RandRange(0, candidates.Num() - 1);
        auto candidate = candidates[candidateId];
        Grid[candidate.first][candidate.second] = GridContent::CHARACTER;
    }
}

void ALevelGeneratorActor::GenerateMaze()
{
    GenerateWalls();
    GenerateExits();
    GenerateCharacters();
}

void ALevelGeneratorActor::SpawnMaze()
{
    FString Maze;
    for (int row = 0; row < GridRows; ++row) {
        for (int column = 0; column < GridColumns; ++column) {
            switch (Grid[row][column]) {
                case GridContent::WALL: {
                    Maze += '#';
                    FIntVector Cell(row, column, 0);
                    SpawnWall(GetCellLocation(Cell));
                    continue;
                }
                case GridContent::EXIT: {
                    Maze += 'O';
                    FIntVector Cell(row, column, 0);
                    SpawnExit(GetCellLocation(Cell));
                    continue;
                }
                case GridContent::CHARACTER: {
                    Maze += 'C';
                    FIntVector Cell(row, column, 0);
                    SpawnCharacter(GetCellLocation(Cell));
                    continue;
                }
                case GridContent::NONE: {
                    Maze += '*';
                    continue;
                }
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
    } while (Grid[RandomRow][RandomColumn] != GridContent::NONE);

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

void ALevelGeneratorActor::SpawnActor(FVector SpawnLocation, FString Name, TSubclassOf<class AActor> ActorClass)
{
    UWorld* const World = GetWorld();
    if (!World) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Instigator;

    FRotator SpawnRotation(0.f, 0.f, 0.f);

    UE_LOG(LogTemp, Warning, TEXT("Spawned %s actor at %s"), *Name, *SpawnLocation.ToString());
    AActor* Actor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);
    FVector Origin;
    FVector BoxExtent;
    Actor->GetActorBounds(true, Origin, BoxExtent);
    FVector Location = Actor->GetActorLocation();
    Location.Z += BoxExtent.Z;
    Actor->SetActorLocation(Location);
    SpawnedActors.Add(Actor);
}

void ALevelGeneratorActor::SpawnWall(FVector SpawnLocation)
{
    SpawnActor(SpawnLocation, TEXT("Wall"), WallActor);
}

void ALevelGeneratorActor::SpawnExit(FVector SpawnLocation)
{
    SpawnActor(SpawnLocation, TEXT("Exit"), ExitActor);
}

void ALevelGeneratorActor::SpawnCharacter(FVector SpawnLocation)
{
    SpawnActor(SpawnLocation, TEXT("Character"), CharacterActor);
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

