// Fill out your copyright notice in the Description page of Project Settings.

#include "Lesson_1.h"
#include "BatteryPickup.h"


ABatteryPickup::ABatteryPickup()
{
    // Set default power level for a battery.
    PowerLevel = 150.0f;
}

void ABatteryPickup::OnPickedUp_Implementation()
{
    // Call parent implementation.
    Super::OnPickedUp_Implementation();
    // Destroy the battery after pickup.
    Destroy();
}