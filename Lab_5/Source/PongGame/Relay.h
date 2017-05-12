// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

/**
 * 
 */
class PONGGAME_API FRelay
{
public:
	FRelay();
	~FRelay();

    void Tick();

    TArray<uint8> Act(const TArray<uint8>& state);

private:

    std::string host;
};
