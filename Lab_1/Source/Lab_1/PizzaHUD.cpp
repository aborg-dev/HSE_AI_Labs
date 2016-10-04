// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "PizzaHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Lab_1GameMode.h"

#include "Engine/Canvas.h"
#include "Engine/Font.h"

APizzaHUD::APizzaHUD()
{
    // Use the RobotoDistanceField font from the engine.
    static ConstructorHelpers::FObjectFinder<UFont>HUDFontOb(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
    HUDFont = HUDFontOb.Object;
}

void APizzaHUD::DrawHUD()
{
    // Get the screen dimensions.
    FVector2D ScreenDimensions = FVector2D(Canvas->SizeX, Canvas->SizeY);

    // Call to the parent versions of DrawHUD.
    Super::DrawHUD();

    auto* MyGameMode = Cast<ALab_1GameMode>(UGameplayStatics::GetGameMode(this));
    FString PendingPizzaOrderCountString = FString::Printf(TEXT("Pending pizza order count %d"), MyGameMode->GetPendingPizzaOrderCount());
    FString DeliveledPizzaOrderCountString = FString::Printf(TEXT("Delivered pizza order count %d"), MyGameMode->GetDeliveredPizzaOrderCount());
    FVector2D PendingSize;
    GetTextSize(PendingPizzaOrderCountString, PendingSize.X, PendingSize.Y, HUDFont);
    DrawText(PendingPizzaOrderCountString, FColor::White, 30, 30, HUDFont);
    DrawText(DeliveledPizzaOrderCountString, FColor::White, 30, 30 + PendingSize.Y + 5, HUDFont);

    // If the game is over.
    if (MyGameMode->GetCurrentState() == ELab_1PlayState::EGameOver)
    {
        // Create a variable for storing the size of printing Game Over.
        FVector2D GameOverSize;
        GetTextSize(TEXT("GAME OVER"), GameOverSize.X, GameOverSize.Y, HUDFont);
        DrawText(TEXT("GAME OVER"), FColor::White, (ScreenDimensions.X - GameOverSize.X) / 2.0f, (ScreenDimensions.Y - GameOverSize.Y) / 2.0f, HUDFont);
    }
}
