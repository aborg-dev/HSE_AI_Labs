// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Relay.h"
#include "RemotePaddleController.generated.h"

using Action = double;
using State = APaddle*;

/**
 * 
 */
UCLASS()
class PONGGAME_API ARemotePaddleController : public AAIController
{
	GENERATED_BODY()

public:

    ARemotePaddleController();

    void BeginPlay();

    void Tick(float DeltaTime);

private:

    Action Communicate(const State& state);

    FRelay relay;

};
