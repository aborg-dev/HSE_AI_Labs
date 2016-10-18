// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "MazeExit.h"


// Sets default values
AMazeExit::AMazeExit()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ExitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExitMesh"));
    RootComponent = ExitMesh;

    ExitIndex = -1;
}

// Called when the game starts or when spawned
void AMazeExit::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AMazeExit::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );
}

void AMazeExit::SetExitIndex(int index)
{
    ExitIndex = index;
}

bool AMazeExit::TryEscape(FVector characterLocation)
{
    float distance = (GetActorLocation() - characterLocation).Size();
    return (distance < EscapeRadius);
}
