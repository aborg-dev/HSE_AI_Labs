// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_1.h"
#include "Lab_1Character.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HouseActor.h"

ALab_1Character::ALab_1Character()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    // Create collection sphere and set it's default radius.
    DeliverySphere = CreateDefaultSubobject<USphereComponent>(TEXT("DeliverySphere"));
    DeliverySphere->SetupAttachment(RootComponent);
    DeliverySphere->SetSphereRadius(200.0f);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 4000.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

    PizzaCapacity = 1;
    PizzaAmount = 0;
}

void ALab_1Character::Tick(float DeltaSeconds)
{
}

int ALab_1Character::GetPizzaAmount() const
{
    return PizzaAmount;
}

int ALab_1Character::GetPizzaCapacity() const
{
    return PizzaCapacity;
}

bool ALab_1Character::TryGrabPizza()
{
    if (PizzaAmount >= PizzaCapacity) {
        return false;
    }
    ++PizzaAmount;
    return true;
}

bool ALab_1Character::TryDeliverPizza(AHouseActor* HouseActor, int OrderNumber)
{
    if (PizzaAmount == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Not enought pizza!"));
        return false;
    }
    if (!HouseActor->WaitsPizzaDelivery()) {
        UE_LOG(LogTemp, Warning, TEXT("House doesn't wait for delivery!"));
        return false;
    }
    // Check overlapping.
    {
        TArray<AActor*> OverlappingActors;
        DeliverySphere->GetOverlappingActors(OverlappingActors);
        if (OverlappingActors.Find(HouseActor) == INDEX_NONE) {
            UE_LOG(LogTemp, Warning, TEXT("Character is not in range for delivery"));
            return false;
        }
    }

    --PizzaAmount;
    HouseActor->OnPizzaDelivered(OrderNumber);
    return true;
}
