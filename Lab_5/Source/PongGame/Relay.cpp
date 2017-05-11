// Fill out your copyright notice in the Description page of Project Settings.

#include "PongGame.h"
#include "Relay.h"

FRelay::FRelay()
{
}

FRelay::~FRelay()
{
}

TArray<uint8> FRelay::Act(const TArray<uint8>& state)
{
     UE_LOG(LogTemp, Warning, TEXT("Sending %d bytes..."), state.Num());
     return state;
}
