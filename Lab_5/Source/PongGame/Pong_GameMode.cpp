// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Pong_GameMode.h"

APong_GameMode::APong_GameMode()
{
    EpisodeStep = 0;
    CpuScore = 0;
    PlayerScore = 0;
    ScreenCapturer = nullptr;
}

void APong_GameMode::BeginPlay()
{
    Super::BeginPlay();

    // Create ScreenCapturer.
    UWorld* const World = GetWorld();
    if (!World) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Instigator;

    FVector SpawnLocation(0.f, 0.f, 0.f);
    FRotator SpawnRotation(0.f, 0.f, 0.f);

    ScreenCapturer = World->SpawnActor<AScreenCapturer>(SpawnLocation, SpawnRotation, SpawnParams);
}
