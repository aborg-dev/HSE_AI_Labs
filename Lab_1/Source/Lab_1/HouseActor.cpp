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

    HouseIndex = -1;

    MaxWaitTime = 20.0f;
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

    for (const auto& Order : PizzaOrders) {
        Order->CurrentWaitTime += DeltaTime;
        if (Order->CurrentWaitTime > MaxWaitTime) {
            bTimeoutReached = true;
        }
    }
}

void AHouseActor::SetHouseIndex(int Index)
{
    HouseIndex = Index;
}

bool AHouseActor::WaitsPizzaDelivery() const
{
    return PizzaOrders.Num() > 0;
}

TSharedRef<FPizzaOrder> AHouseActor::OrderPizzaDelivery(int OrderNumber)
{
    if (HouseIndex == -1) {
        UE_LOG(LogTemp, Warning, TEXT("House index is not set, failed to create pizza order"));
        check(HouseIndex != -1);
    }

    TSharedRef<FPizzaOrder> Order(new FPizzaOrder(OrderNumber, HouseIndex, 1));
    PizzaOrders.Add(Order);
    TogglePizzaDeliveryHighlight();
    return Order;
}

void AHouseActor::OnPizzaDelivered(int OrderNumber)
{
    if (!PizzaOrders.Num()) {
        UE_LOG(LogTemp, Warning, TEXT("House doesn't wait for delivery!"));
        return;
    }

    int Index = 0;
    for (; Index < PizzaOrders.Num(); ++Index) {
        if (PizzaOrders[Index]->OrderNumber == OrderNumber) {
            break;
        }
    }

    if (Index == PizzaOrders.Num()) {
        UE_LOG(LogTemp, Warning, TEXT("Order %d is not found"), OrderNumber);
        return;
    }

    PizzaOrders.RemoveAtSwap(Index);
    if (!PizzaOrders.Num()) {
        TogglePizzaDeliveryHighlight();
    }
}

void AHouseActor::TogglePizzaDeliveryHighlight()
{
    if (PizzaDeliveryHighlightComponent && PizzaDeliveryHighlightComponent->Template) {
        if (PizzaOrders.Num()) {
            //PizzaDeliveryHighlightComponent->Activate();
        } else {
            //PizzaDeliveryHighlightComponent->Deactivate();
        }
    }
}

bool AHouseActor::TimeoutReached() const
{
    return bTimeoutReached;
}

float AHouseActor::GetTimeLeft() const
{
    float MinTimeLeft = MaxWaitTime;
    for (const auto& Order : PizzaOrders) {
        MinTimeLeft = fmin(MinTimeLeft, MaxWaitTime - Order->CurrentWaitTime);
    }
    return MinTimeLeft;
}
