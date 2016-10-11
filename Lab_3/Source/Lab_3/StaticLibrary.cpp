// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_3.h"
#include "StaticLibrary.h"

bool UStaticLibrary::Trace(
    UWorld* World,
    AActor* ActorToIgnore,
    const FVector& Start,
    const FVector& End,
    FHitResult& HitOut,
    ECollisionChannel CollisionChannel,
    bool ReturnPhysMat)
{
    if(!World) {
        return false;
    }

    FCollisionQueryParams TraceParams(FName(TEXT("Trace")), true, ActorToIgnore);
    TraceParams.bTraceComplex = true;
    // TraceParams.bTraceAsyncScene = true;
    TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

    // Ignore Actors
    TraceParams.AddIgnoredActor(ActorToIgnore);

    // Re-initialize hit info
    HitOut = FHitResult(ForceInit);

    // Trace!
    World->LineTraceSingleByChannel(
        HitOut,
        Start,
        End,
        CollisionChannel,
        TraceParams);

    // Hit any Actor?
    return HitOut.bBlockingHit;
}
