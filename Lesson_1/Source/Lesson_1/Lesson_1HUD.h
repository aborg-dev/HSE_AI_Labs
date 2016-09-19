// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "Lesson_1HUD.generated.h"

/**
 * 
 */
UCLASS()
class LESSON_1_API ALesson_1HUD : public AHUD
{
	GENERATED_BODY()

public:
    ALesson_1HUD();

    // Store the font used for messages.
    UPROPERTY()
    UFont* HUDFont;

    // Override draw function to show custom messages.
    virtual void DrawHUD() override;
};
