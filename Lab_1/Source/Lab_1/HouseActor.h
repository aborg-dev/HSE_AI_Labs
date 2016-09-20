// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HouseActor.generated.h"

UCLASS()
class LAB_1_API AHouseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHouseActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    bool WaitsPizzaDelivery() const;

    void OrderPizzaDelivery();

    // Called when pizza is delivered.
    void OnPizzaDelivered();

    void TogglePizzaDeliveryHighlight();

    // Component for handling collisions.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    USphereComponent* BaseCollisionComponent;

    // Component that represents house in the real world.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    UStaticMeshComponent* HouseMesh;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    UParticleSystemComponent *PizzaDeliveryHighlightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Delivery)
    float MaxWaitTime;

    bool TimeoutReached() const;

private:
    // Is true if house ordered some delivery.
    bool bWaitsPizzaDelivery;

    float CurrentWaitTime;

    bool bTimeoutReached;
};
