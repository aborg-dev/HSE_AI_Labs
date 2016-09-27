// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PizzaOrder.h"
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

    void SetHouseIndex(int Index);

    bool WaitsPizzaDelivery() const;

    // Called when new order for the house is spawned.
    TSharedRef<FPizzaOrder> OrderPizzaDelivery(int OrderNumber);

    // Called when pizza is delivered.
    void OnPizzaDelivered(int OrderNumber);

    // Returns remaining time of the longest waiting order.
    UFUNCTION(BlueprintCallable, Category = Delivery)
    float GetTimeLeft() const;

    // True if timeout for at least one of the orders is reached.
    bool TimeoutReached() const;

    // Component for handling collisions.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    USphereComponent* BaseCollisionComponent;

    // Component that represents house in the real world.
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    UStaticMeshComponent* HouseMesh;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = House)
    UParticleSystemComponent *PizzaDeliveryHighlightComponent;

    // Max time the order is ready to wait.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Delivery)
    float MaxWaitTime;

private:
    int HouseIndex;

    TArray<TSharedRef<FPizzaOrder>> PizzaOrders;

    bool bTimeoutReached;

    void TogglePizzaDeliveryHighlight();
};
