// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "ScreenCapturer.h"
#include "Pong_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class PONGGAME_API APong_GameMode : public AGameMode
{
	GENERATED_BODY()

public:

    APong_GameMode();

    void BeginPlay();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    int EpisodeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    int CpuScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    int PlayerScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    FVector2D BallPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    FVector2D BallSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    FVector2D CpuPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    FVector2D PlayerPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    AScreenCapturer* ScreenCapturer;

};
