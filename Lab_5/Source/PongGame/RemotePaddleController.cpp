// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Paddle.h"
#include "RemotePaddleController.h"



ARemotePaddleController::ARemotePaddleController()
{
}

void ARemotePaddleController::BeginPlay()
{
    Super::BeginPlay();
}

void ARemotePaddleController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* pawn = GetPawn();
    if (!pawn) {
        return;
    }

    APaddle* paddle = Cast<APaddle>(GetPawn());
    if (!paddle) {
        return;
    }

    TotalTime += DeltaTime;
    while (TotalTime > 1) {
        TotalTime -= 2;
    }
    paddle->MovementDirection = TotalTime;

    Relay.Tick();
}
