// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "HouseActor.h"


// Sets default values
AHouseActor::AHouseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create collision component.
    BaseCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));

    // Set collision component as a root component.
    RootComponent = BaseCollisionComponent;

    // Create pickup mesh component.
    HouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HouseMesh"));

    // Attach mesh to root component.
    HouseMesh->SetupAttachment(RootComponent);

    PizzaDeliveryHighlightComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PizzaDeliveryHighlight"));
    PizzaDeliveryHighlightComponent->SetupAttachment(HouseMesh);
    PizzaDeliveryHighlightComponent->bAutoActivate = false;
    PizzaDeliveryHighlightComponent->SetRelativeLocation(FVector(-20.0f, 0.0f, 20.0f));
    static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/GameContent/Particles/P_Fire.P_Fire"));
    if (ParticleAsset.Succeeded())
    {
        PizzaDeliveryHighlightComponent->SetTemplate(ParticleAsset.Object);
    }

    bWaitsPizzaDelivery = false;

    MaxWaitTime = 10.0f;
    bTimeoutReached = false;
}

// Called when the game starts or when spawned
void AHouseActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHouseActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

    if (bWaitsPizzaDelivery) {
        CurrentWaitTime += DeltaTime;
        if (CurrentWaitTime > MaxWaitTime) {
            bTimeoutReached = true;
        }
    }
}

bool AHouseActor::WaitsPizzaDelivery() const
{
    return bWaitsPizzaDelivery;
}

void AHouseActor::OrderPizzaDelivery()
{
    bWaitsPizzaDelivery = true;
    TogglePizzaDeliveryHighlight();
}

void AHouseActor::OnPizzaDelivered()
{
    bWaitsPizzaDelivery = false;
    TogglePizzaDeliveryHighlight();
}

void AHouseActor::TogglePizzaDeliveryHighlight()
{
    if (PizzaDeliveryHighlightComponent && PizzaDeliveryHighlightComponent->Template)
    {
        PizzaDeliveryHighlightComponent->ToggleActive();
    }
}

bool AHouseActor::TimeoutReached() const
{
    return bTimeoutReached;
}

float AHouseActor::GetTimeLeft() const
{
    return MaxWaitTime - CurrentWaitTime;
}
