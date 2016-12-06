// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "ScreenCapturer.h"

#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "SceneViewport.h"

// Sets default values
AScreenCapturer::AScreenCapturer()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    ScreenshotPeriod = 0.1;
    ScreenshotTimer = 0.0;
    Height = 0;
    Width = 0;
}

// Called when the game starts or when spawned
void AScreenCapturer::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AScreenCapturer::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );

    if (CaptureScreenshot(&Screenshot)) {
        UE_LOG(LogTemp, Warning, TEXT("Screenshot taken, dimensions: (%d, %d), timer: %f"),
               Height,
               Width,
               DeltaTime);
    }
}

/**
 * Capture a screenshot from this actor's viewport.
 *
 * @param data a float array, filled with the screenshot data in [Y,X,color] order.
 * @returns true if successful
 */
bool AScreenCapturer::CaptureScreenshot(TArray<float>* data)
{
    FlushRenderingCommands();

    if (GEngine == NULL) {
        UE_LOG(LogTemp, Warning, TEXT("GEngine null"));
        return false;
    }
    if (GEngine->GameViewport == NULL) {
        UE_LOG(LogTemp, Warning, TEXT("GameViewport null"));
        return false;
    }
    if (GEngine->GameViewport->Viewport == NULL) {
        UE_LOG(LogTemp, Warning, TEXT("Viewport null"));
        return false;
    }

    FViewport* Viewport = GEngine->GameViewport->Viewport;
    TArray<FColor> Bitmap;

    int X = Viewport->GetSizeXY().X;
    int Y = Viewport->GetSizeXY().Y;
    data->SetNumUninitialized(X * Y * 3);

    TSharedPtr<SWidget> ViewportPtr = GEngine->GameViewport->GetGameViewportWidget();

    bool bScreenshotSuccessful = false;
    FIntRect SizeRect(0, 0, X, Y);
    if (ViewportPtr.IsValid() && FSlateApplication::IsInitialized()) {
        FIntVector OutSize;
        TSharedRef<SWidget> ViewportRef = ViewportPtr.ToSharedRef();
        bScreenshotSuccessful = FSlateApplication::Get().TakeScreenshot(
            ViewportRef, SizeRect, Bitmap, OutSize);
    } else {
        bScreenshotSuccessful = GetViewportScreenShot(Viewport, Bitmap, SizeRect);
    }

    if (bScreenshotSuccessful) {
        if (Bitmap.Num() != X * Y) {
            UE_LOG(LogTemp, Warning,
                   TEXT("Screenshot bitmap had the wrong number of elements: %d"),
                   Bitmap.Num());
            return false;
        }
        Height = X;
        Width = Y;

        float* values = data->GetData();
        for (const FColor& color : Bitmap) {
            *values++ = color.R / 255.0f;
        }
        for (const FColor& color : Bitmap) {
            *values++ = color.G / 255.0f;
        }
        for (const FColor& color : Bitmap) {
            *values++ = color.B / 255.0f;
        }
    }

    return bScreenshotSuccessful;
}
