// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Lab_1Character.generated.h"

class AHouseActor;

UCLASS(Blueprintable)
class ALab_1Character : public ACharacter
{
	GENERATED_BODY()

public:
	ALab_1Character();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Delivery)
    int PizzaCapacity;

    int GetPizzaCapacity() const;

    int GetPizzaAmount() const;

    // Tries to get pizza from pizza storage. Returns true on success and false otherwise.
    bool TryGrabPizza();

    // Tries to deliver pizza to specified house. Returns true on success and false otherwise.
    bool TryDeliverPizza(AHouseActor* HouseActor, int OrderNumber);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

    /** Area where actor can deliver pizza */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Delivery, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* DeliverySphere;

    int PizzaAmount;
};

