// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lesson_1.h"
#include "Lesson_1Character.h"
#include "Lesson_1GameMode.h"
#include "BatteryPickup.h"

#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ALesson_1Character

ALesson_1Character::ALesson_1Character()
{
    // Set initial power level of the character.
    PowerLevel = 2000.0f;

    // Set the power-speed multiplier and base speed of the character.
    SpeedFactor = 0.75f;
    BaseSpeed = 10.0f;

    // Create collection sphere and set it's default radius.
    CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
    CollectionSphere->SetupAttachment(RootComponent);
    CollectionSphere->SetSphereRadius(200.0f);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALesson_1Character::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    // Bind CollectPickups key mapping to call CollectBatteries function.
    InputComponent->BindAction("CollectPickups", IE_Pressed, this, &ALesson_1Character::CollectBatteries);

	InputComponent->BindAxis("MoveForward", this, &ALesson_1Character::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ALesson_1Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ALesson_1Character::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ALesson_1Character::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(IE_Pressed, this, &ALesson_1Character::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &ALesson_1Character::TouchStopped);
}


void ALesson_1Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void ALesson_1Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void ALesson_1Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ALesson_1Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ALesson_1Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ALesson_1Character::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ALesson_1Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Character speed is bounded with BaseSpeed from below and linearly depends on the power level.
    GetCharacterMovement()->MaxWalkSpeed = SpeedFactor * PowerLevel + BaseSpeed;
}

void ALesson_1Character::CollectBatteries()
{
    // Don't collect batteries when the game is over.
    ALesson_1GameMode* MyGameMode = Cast<ALesson_1GameMode>(UGameplayStatics::GetGameMode(this));
    if (MyGameMode->GetCurrentState() == ELesson_1PlayState::EGameOver) {
        return;
    }

    // Stores total collected power from one sweep of CollectBatteries call.
    float BatteryPower = 0.0f;

    // First we get all the actors that are close enough to the character (overlap with CollectionSphere).
    TArray<AActor*> CollectedActors;
    CollectionSphere->GetOverlappingActors(CollectedActors);
    for (int i = 0; i < CollectedActors.Num(); ++i) {
        // We iterate over every neighboring actor and check,
        // whether it is a battery and it is still active.
        auto* TestBattery = Cast<ABatteryPickup>(CollectedActors[i]);
        if (TestBattery && !TestBattery->IsPendingKill() && TestBattery->bIsActive) {
            // We found a battery and we collect it's power and deactivate it.
            BatteryPower += TestBattery->PowerLevel;
            TestBattery->bIsActive = false;
            TestBattery->OnPickedUp();
        }
    }

    // Finally, if we managed to collect anything, we power up the character by the collected ammount.
    if (BatteryPower > 0.0f) {
        PowerLevel += BatteryPower;
        PowerUp(BatteryPower);
    }
}
