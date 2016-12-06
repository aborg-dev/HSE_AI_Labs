// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ScreenCapturer.generated.h"

UCLASS()
class PONGGAME_API AScreenCapturer : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AScreenCapturer();

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick( float DeltaSeconds ) override;

    UPROPERTY(EditAnywhere, Category = Screenshot)
    TArray<float> Screenshot;

    UPROPERTY(EditAnywhere, Category = Screenshot)
    int Height;

    UPROPERTY(EditAnywhere, Category = Screenshot)
    int Width;

    UPROPERTY(EditAnywhere, Category = Screenshot)
    float ScreenshotPeriod;

private:
    bool CaptureScreenshot(TArray<float>* data);

    float ScreenshotTimer;

};
