// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "PizzaHUD.generated.h"

/**
 * 
 */
UCLASS()
class LAB_1_API APizzaHUD : public AHUD
{
	GENERATED_BODY()

public:
    APizzaHUD();

    // Store the font used for messages.
    UPROPERTY()
    UFont* HUDFont;

    // Override draw function to show custom messages.
    virtual void DrawHUD() override;
};
