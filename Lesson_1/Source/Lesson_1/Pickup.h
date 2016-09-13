// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class LESSON_1_API APickup : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    // Is true if pickup is active and false otherwise.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pickup)
    bool bIsActive;

    // Component for handling collisions.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Pickup)
    USphereComponent* BaseCollisionComponent;

    // Component that represents pickup in the real world.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Pickup)
    UStaticMeshComponent* PickupMesh;

    // Called when pickup is collected.
    UFUNCTION(BlueprintNativeEvent)
    void OnPickedUp();
};
