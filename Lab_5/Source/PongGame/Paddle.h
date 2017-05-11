// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "Pong_GameMode.h"
#include "Paddle.generated.h"

UCLASS()
class PONGGAME_API APaddle : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APaddle();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    float PaddleSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
    APong_GameMode* GameMode;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



};
