// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Paddle.h"
#include "RemotePaddleController.h"

#include <msgpack.hpp>
#include <sstream>
#include <stdint.h>


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

template <typename It>
int decode_dir(It begin, It end) {
    int res = 0;
    int base = 1 << 8;
    while (begin != end) {
        res *= base;
        res += *begin;
        ++begin;
    }
    return res - 1;
}

struct Point {
    Point(float x_, float y_): x(x_), y(y_) { }
    Point(FVector2D v): Point(v.X, v.Y) { }
    Point() { }

    float x;
    float y;

    MSGPACK_DEFINE(x, y);
};

struct Message {
    int episodeStep;
    int cpuScore;
    int playerScore;

    Point ballPosition;
    Point ballSpeed;
    Point cpuPosition;
    Point playerPosition;

    int height;
    int width;
    std::vector<uint8_t> screen;

    MSGPACK_DEFINE(
            episodeStep, cpuScore, playerScore,
            ballPosition, ballSpeed, cpuPosition, playerPosition,
            height, width, screen);
};

Action ARemotePaddleController::Communicate(const State& state) {
    auto* gameMode = state->GameMode;
    auto screenshot = gameMode->ScreenCapturer->Screenshot;

    if (screenshot.size() > 100000) {
        UE_LOG(LogTemp, Warning, TEXT("Screenshot is too large: %d bytes"), screenshot.size());
        return 0.0;
    }

    Message message;
    message.episodeStep = gameMode->EpisodeStep;
    message.cpuScore = gameMode->CpuScore;
    message.playerScore = gameMode->PlayerScore;

    message.ballPosition = gameMode->BallPosition;
    message.ballSpeed = gameMode->BallSpeed;
    message.cpuPosition = gameMode->CpuPosition;
    message.playerPosition = gameMode->PlayerPosition;

    message.height = gameMode->ScreenCapturer->Height;
    message.width = gameMode->ScreenCapturer->Width;
    message.screen = screenshot;

    try {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, message);

        auto action = relay.Act(sbuf.data(), sbuf.size());

        int dst = decode_dir(action.begin(), action.end());

        //msgpack::object_handle oh = msgpack::unpack(action.data(), action.size());
        //msgpack::object deserialized = oh.get();

        //deserialized.convert(dst);

        return dst;
    } catch (std::exception& e) {
        FString error_message(e.what());
        UE_LOG(LogTemp, Warning, TEXT("Caught an exception: %s"), *error_message);

        return 0.0;
    }
}
