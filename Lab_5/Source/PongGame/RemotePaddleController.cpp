// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Paddle.h"
#include "RemotePaddleController.h"

#include <msgpack.hpp>
#include <sstream>


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

    paddle->MovementDirection = Communicate(paddle);
}

Action ARemotePaddleController::Communicate(const State& state) {
    int EpisodeStep = state->GameMode->EpisodeStep;
    int CpuScore = state->GameMode->CpuScore;
    int PlayerScore = state->GameMode->PlayerScore;

    try {
        msgpack::type::tuple<int, int, int> src(EpisodeStep, CpuScore, PlayerScore);
        std::stringstream buffer;
        msgpack::pack(buffer, src);

        auto action = relay.Act(buffer.str());

        msgpack::object_handle oh = msgpack::unpack(action.data(), action.size());
        msgpack::object deserialized = oh.get();
        float dst;
        deserialized.convert(dst);

        UE_LOG(LogTemp, Warning, TEXT("Deserialized value: %f"), dst);

        return dst;
    } catch (std::exception& e) {
        FString message(e.what());
        UE_LOG(LogTemp, Warning, TEXT("Caught an exception: %s"), *message);

        return 0.0;
    }
}
