// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MazeHUD.generated.h"

/**
 * 
 */
UCLASS()
class LAB_3_API AMazeHUD : public AHUD
{
    GENERATED_BODY()

public:
    AMazeHUD();

    // Store the font used for messages.
    UPROPERTY()
    UFont* HUDFont;

    // Override draw function to show custom messages.
    virtual void DrawHUD() override;
};
