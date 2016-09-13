// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup.h"
#include "BatteryPickup.generated.h"

/**
 * Class to represent a battery pickup.
 */
UCLASS()
class LESSON_1_API ABatteryPickup : public APickup
{
	GENERATED_BODY()

public:
    ABatteryPickup();

    // The ammount of power stored in the battery.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Power)
    float PowerLevel;

    // Override base class method implementation to customize the battery behavior.
    void OnPickedUp_Implementation() override;

};
