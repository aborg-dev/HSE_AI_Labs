// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "StaticLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LAB_3_API UStaticLibrary : public UObject
{
    GENERATED_BODY()

public:
    static bool Trace(
        UWorld* World,
        AActor* ActorToIgnore,
        const FVector& Start,
        const FVector& End,
        FHitResult& HitOut,
        ECollisionChannel CollisionChannel = ECC_WorldStatic,
        bool ReturnPhysMat = false);
};
