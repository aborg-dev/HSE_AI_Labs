// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class PONGGAME_API FRelay
{
public:
	FRelay();
	~FRelay();

    TArray<uint8> Act(const TArray<uint8>& state);
};
