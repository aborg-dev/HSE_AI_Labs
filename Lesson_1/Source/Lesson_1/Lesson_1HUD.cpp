// Fill out your copyright notice in the Description page of Project Settings.

#include "Lesson_1.h"
#include "Lesson_1HUD.h"
#include "Lesson_1GameMode.h"
#include "Lesson_1Character.h"

#include "Kismet/GameplayStatics.h"

#include "Engine/Canvas.h"
#include "Engine/Font.h"

ALesson_1HUD::ALesson_1HUD()
{
    //Use the RobotoDistanceField font from the engine
    static ConstructorHelpers::FObjectFinder<UFont>HUDFontOb(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
    HUDFont = HUDFontOb.Object;
}


void ALesson_1HUD::DrawHUD()
{
    //Get the screen dimensions
    FVector2D ScreenDimensions = FVector2D(Canvas->SizeX, Canvas->SizeY);

    //Call to the parent versions of DrawHUD
    Super::DrawHUD();

    //Get the character and print its power level
    ALesson_1Character* MyCharacter = Cast<ALesson_1Character>(UGameplayStatics::GetPlayerPawn(this, 0));
    FString PowerLevelString = FString::Printf(TEXT("%10.1f"), FMath::Abs(MyCharacter->PowerLevel));
    DrawText(PowerLevelString, FColor::White, 50, 50, HUDFont);

    ALesson_1GameMode* MyGameMode = Cast<ALesson_1GameMode>(UGameplayStatics::GetGameMode(this));
    //if the game is over
    if (MyGameMode->GetCurrentState() == ELesson_1PlayState::EGameOver)
    {
        // create a variable for storing the size of printing Game Over
        FVector2D GameOverSize;
        GetTextSize(TEXT("GAME OVER"), GameOverSize.X, GameOverSize.Y, HUDFont);
        DrawText(TEXT("GAME OVER"), FColor::White, (ScreenDimensions.X - GameOverSize.X) / 2.0f, (ScreenDimensions.Y - GameOverSize.Y) / 2.0f, HUDFont);
        
    }
}