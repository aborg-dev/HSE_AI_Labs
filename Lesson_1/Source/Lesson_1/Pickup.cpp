// Fill out your copyright notice in the Description page of Project Settings.

#include "Lesson_1.h"
#include "Pickup.h"


// Sets default values
APickup::APickup()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Pickup is active by default.
    bIsActive = true;

    // Create collision component.
    BaseCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));

    // Set collision component as a root component.
    RootComponent = BaseCollisionComponent;

    // Create pickup mesh component.
    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));

    // Turn on phy  sics (e.g. gravity, collisions).
    PickupMesh->SetSimulatePhysics(true);

    // Attach mesh to root component.
    PickupMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void APickup::OnPickedUp_Implementation()
{
    // There is no default behavior for Pickup base class.
}
