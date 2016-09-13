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
	
	UPROPERTY()
    UFont* HUDFont;

    virtual void DrawHUD() override;
	
};
